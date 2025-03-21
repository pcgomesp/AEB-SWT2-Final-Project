#include <stdio.h>
#include <math.h>
#include <time.h>
#include <unistd.h>

// Function for calculate acceleration
float accel_calc(float spd) {
    static float prev_spd = 0.0;
    static struct timespec start_time = {0, 0};
    struct timespec current_time;
    double elapsed_time;
    float accel = 0.0;

    clock_gettime(CLOCK_REALTIME, &current_time);
    printf("Current time: %ld.%09ld\n", current_time.tv_sec, current_time.tv_nsec);

    if (prev_spd == 0.0) {
        prev_spd = spd;
        start_time = current_time;
        return 0.0;
    }

    else {
        elapsed_time = (double)(current_time.tv_sec - start_time.tv_sec) 
                        + (double)(current_time.tv_nsec - start_time.tv_nsec) / 1e9;
        if (elapsed_time < 0.01) return 0.0; // To avoid division by zero
        printf("Elapsed time: %f\n", elapsed_time);
        
        accel = ((spd - prev_spd) / 3.6) / elapsed_time; // acceleration in m/s^2
        start_time = current_time;
        prev_spd = spd;
        
        return accel;
    } 
}

// Function for calculating the time to collision (TTC)
float ttc_calc(float dis_rel, float spd_rel) {
    //static float prev_spd = 0.0; // scratch
    // Quadratic equation coefficients - UVM (Uniformly Variable Motion)
    float a, b, c, ttc, delta;
    
    a = accel_calc(spd_rel);
    printf("Acceleration: %f\n", a);
    b = spd_rel / 3.6;
    printf("Speed: %f\n", spd_rel);
    c = dis_rel;
    printf("Distance: %f\n\n", dis_rel);

    if (a == 0) return ttc = dis_rel / spd_rel;

    // Calculating the discriminant
    delta = b * b + 2 * a * c;
    printf("Delta: %f\n\n", delta);
    if (delta < 0) return -1.0; // No real roots

    else if (delta == 0) return -b / a;

    else {
        // calculating the root
        ttc = (-b + sqrt(delta)) / a;

        // returning the positive root
        return ttc;
    }
}

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
