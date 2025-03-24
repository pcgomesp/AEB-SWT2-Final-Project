// Necessary libraries
#include "../inc/ttc_control.h"
#include <stdio.h>
#include <unistd.h> // to use sleep

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
    float speed1 = 10.0;  // 10 km/h
    float speed2 = 20.0;  // 20 km/h

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
    float dist = 100.0;  // 100 metros
    float speed = 20.0;  // 20 km/h
    float ttc = ttc_calc(dist, speed);
    sleep(2);
    ttc = ttc_calc(dist, speed + 30.0);
    printf("Tempo para colisão com distância de %.1f metros e velocidade de %.1f km/h: %0.2f segundos\n", dist, speed, ttc);

    // Teste com outra velocidade
    dist = 50.0;  // 50 metros
    speed = 10.0; // 10 km/h
    ttc = ttc_calc(dist, speed);
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
    // Variáveis de entrada para o teste
    bool enable_aeb;
    bool alarm_cluster;
    bool enable_breaking;
    float spd;
    float dist;

    // Teste 1: TTC > 2.0 (sem alarme ou frenagem)
    enable_aeb = true;
    spd = 30.0;  // 30 km/h
    dist = 50.0; // 50 metros
    aeb_control(&enable_aeb, &alarm_cluster, &enable_breaking, &spd, &dist);

    // Verifique se o alarme e a frenagem não foram ativados
    printf("Test 1 - TTC > 2.0\n");
    printf("Expected: No alarm, no braking\n");
    printf("Alarm: %s, Braking: %s\n", alarm_cluster ? "Triggered" : "Not triggered", enable_breaking ? "Enabled" : "Not enabled");

    // Teste 2: TTC entre 1.0 e 2.0 (alarme acionado, mas não frenagem)
    dist = 20.0; // 20 metros
    spd = 30.0;  // 30 km/h
    aeb_control(&enable_aeb, &alarm_cluster, &enable_breaking, &spd, &dist);

    // Verifique se o alarme foi ativado, mas a frenagem não
    printf("\nTest 2 - 1.0 < TTC < 2.0\n");
    printf("Expected: Alarm triggered, no braking\n");
    printf("Alarm: %s, Braking: %s\n", alarm_cluster ? "Triggered" : "Not triggered", enable_breaking ? "Enabled" : "Not enabled");

    // Teste 3: TTC < 1.0 (alarme e frenagem acionados)
    dist = 10.0; // 10 metros
    spd = 30.0;  // 30 km/h
    aeb_control(&enable_aeb, &alarm_cluster, &enable_breaking, &spd, &dist);

    // Verifique se o alarme e a frenagem foram ativados
    printf("\nTest 3 - TTC < 1.0\n");
    printf("Expected: Alarm triggered, braking enabled\n");
    printf("Alarm: %s, Braking: %s\n", alarm_cluster ? "Triggered" : "Not triggered", enable_breaking ? "Enabled" : "Not enabled");

    // Teste 4: TTC < 1.0 com AEB desabilitado (sem alarme nem frenagem)
    enable_aeb = false;
    dist = 10.0; // 10 metros
    spd = 30.0;  // 30 km/h
    aeb_control(&enable_aeb, &alarm_cluster, &enable_breaking, &spd, &dist);

    // Verifique se o alarme e a frenagem não foram ativados, já que o AEB está desativado
    printf("\nTest 4 - AEB Disabled\n");
    printf("Expected: No alarm, no braking\n");
    printf("Alarm: %s, Braking: %s\n", alarm_cluster ? "Triggered" : "Not triggered", enable_breaking ? "Enabled" : "Not enabled");
}

int main() {
    test_accel_calc();
    test_ttc_calc();
    test_aeb_control();

    return 0;
}
