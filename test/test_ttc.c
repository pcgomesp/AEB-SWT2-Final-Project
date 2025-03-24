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

int main() {
    test_accel_calc();
    test_ttc_calc();

    return 0;
}
