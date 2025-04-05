// Necessary libraries
#include "../inc/ttc_control.h"
#include <stdio.h>
#include <unistd.h> // to use sleep

void setUp()
{
    // set stuff up here
}

void tearDown()
{
    // clean stuff up here
}

/**
 * @brief Test the accel_calc function.
 *
 * This function tests the `accel_calc` function by simulating different speeds 
 * and calculating the acceleration between them. It prints the calculated 
 * acceleration in m/s² for various speed values at different time intervals.
 * 
 * It performs the following steps:
 * - Tests the acceleration calculation between two speeds (10 km/h and 20 km/h).
 * - Calls the `accel_calc` function with different speeds and prints the results.
 * - Waits 2 seconds between calls to simulate time intervals.
 */
void test_accel_calc() {
    printf("Testando função accel_calc\n");
    double speed1 = 10.0;  // 10 km/h
    double speed2 = 20.0;  // 20 km/h

    accel_calc(speed1);
    sleep(2);
    printf("Aceleração entre %0.1f km/h e %0.1f km/h: %0.4f m/s^2\n", 
            speed1, speed2, accel_calc(speed2));
    sleep(2);
    printf("Aceleração entre %0.1f km/h e %0.1f km/h: %0.4f m/s^2\n", 
            speed2, speed1, accel_calc(speed1));
    printf("\n");
}

/**
 * @brief Test the ttc_calc function.
 *
 * This function tests the `ttc_calc` function by simulating different distances 
 * and speeds, and then calculates the time to collision (TTC). The function 
 * prints the calculated TTC values for different input parameters.
 * 
 * It performs the following steps:
 * - Tests the TTC with a 100-meter distance and a 20 km/h speed.
 * - Tests TTC with different speeds (adding 30 km/h).
 * - Tests TTC with a 50-meter distance and a 10 km/h speed, and checks for negative results.
 * 
 * @note If the calculated TTC is negative, it indicates that there is no real solution 
 *       for the given inputs (i.e., no real roots in the quadratic equation).
 */
void test_ttc_calc() {
    printf("Testando função ttc_calc\n");

    // Testes com distância e velocidade
    double dist = 100.0;  // 100 metros
    double speed = 20.0;  // 20 km/h
    double acel  = 0.0;  // 20 m/s²
    double ttc = ttc_calc(dist, speed, acel);
    sleep(2);
    ttc = ttc_calc(dist, speed + 30.0, acel);
    printf("Tempo para colisão com distância de %.1f metros e velocidade de %.1f km/h: %0.2f segundos\n", dist, speed, ttc);

    // Teste com outra velocidade
    dist = 50.0;  // 50 metros
    speed = 10.0; // 10 km/h
    acel  = 0.0;
    ttc = ttc_calc(dist, speed, acel);
    if (ttc >= 0) {
        printf("Tempo para colisão com distância de %.1f metros e velocidade de %.1f km/h: %0.2f segundos\n", dist, speed, ttc);
    } else {
        printf("Não há solução real para TTC com essas entradas\n");
    }

    printf("\n\n");
}

/**
 * @brief Test the behavior of the Autonomous Emergency Braking (AEB) control function.
 *
 * This function tests the `aeb_control` function with different values for vehicle speed (spd) and 
 * relative distance (dist) to simulate various scenarios of time-to-collision (TTC) and check 
 * whether the alarm and braking systems are triggered as expected. The function performs the 
 * following tests:
 * 
 * - **Test 1**: TTC > 2.0 seconds (expected result: no alarm, no braking)
 * - **Test 2**: 1.0 < TTC < 2.0 seconds (expected result: alarm triggered, no braking)
 * - **Test 3**: TTC < 1.0 second (expected result: alarm triggered, braking enabled)
 * - **Test 4**: AEB disabled, TTC < 1.0 second (expected result: no alarm, no braking)
 * 
 * Each test prints the expected result and the actual outcome for the alarm and braking flags.
 */
void test_aeb_control() {
    // Input variables for the test
    bool enable_aeb;
    bool alarm_cluster;
    bool enable_breaking;
    bool lk_seatbelt;
    bool lk_doors;
    double spd;
    double dist;
    double acel;
    double delta_spd;

    // Test 1: TTC > 2.0 (no alarm, no braking)
    enable_aeb = true;
    spd = 30.0;  // 30 km/h
    dist = 50.0; // 50 meters
    acel  = 0.0;
    aeb_control(&enable_aeb, &alarm_cluster, &enable_breaking, &lk_seatbelt, &lk_doors, &spd, &dist, &acel);
    delta_spd = 0.001;
    spd -= delta_spd;
    printf("Velocidade: %.6f km/h\n", spd);
    aeb_control(&enable_aeb, &alarm_cluster, &enable_breaking, &lk_seatbelt, &lk_doors, &spd, &dist, &acel);
    
    // Check if the alarm and braking are not triggered
    printf("Test 1 - TTC > 2.0\n");
    printf("Expected: No alarm, no braking\n");
    printf("Alarm: %s, Braking: %s\n", alarm_cluster ? "Triggered" : "Not triggered", enable_breaking ? "Enabled" : "Not enabled");

    // Test 2: 1.0 < TTC < 2.0 (alarm triggered, no braking)
    dist = 12.5; // 12.5 meters
    spd += delta_spd; // 30.0 km/h
    aeb_control(&enable_aeb, &alarm_cluster, &enable_breaking, &lk_seatbelt, &lk_doors, &spd, &dist, &acel);

    // Check if the alarm was triggered, but braking was not
    printf("\nTest 2 - 1.0 < TTC < 2.0\n");
    printf("Expected: Alarm triggered, no braking\n");
    printf("Alarm: %s, Braking: %s\n", alarm_cluster ? "Triggered" : "Not triggered", enable_breaking ? "Enabled" : "Not enabled");

    // Test 3: TTC < 1.0 (alarm and braking triggered)
    dist = 5.0; // 5 meters
    spd += (5*delta_spd); // 30.002 km/h
    aeb_control(&enable_aeb, &alarm_cluster, &enable_breaking, &lk_seatbelt, &lk_doors, &spd, &dist, &acel);

    // Check if the alarm and braking were both triggered
    printf("\nTest 3 - TTC < 1.0\n");
    printf("Expected: Alarm triggered, braking enabled\n");
    printf("Alarm: %s, Braking: %s\n", alarm_cluster ? "Triggered" : "Not triggered", enable_breaking ? "Enabled" : "Not enabled");

    // Test 4: TTC < 1.0 with AEB disabled (no alarm, no braking)
    enable_aeb = false;
    dist = 10.0; // 10 meters
    spd = 30.0;  // 30 km/h
    aeb_control(&enable_aeb, &alarm_cluster, &enable_breaking, &lk_seatbelt, &lk_doors, &spd, &dist, &acel);

    // Check if the alarm and braking were not triggered because AEB is disabled
    printf("\nTest 4 - AEB Disabled\n");
    printf("Expected: No alarm, no braking\n");
    printf("Alarm: %s, Braking: %s\n", alarm_cluster ? "Triggered" : "Not triggered", enable_breaking ? "Enabled" : "Not enabled");
}

int main() {
    // test_accel_calc();
    // test_ttc_calc();
    // test_aeb_control();

    return 0;
}
