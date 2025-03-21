// Necessary libraries
#include <stdio.h>
#include <stdlib.h>
#include <mqueue.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>
#include "../inc/mq_utils.h"
#include "../inc/constants.h"


// Structure for storing sensor data
typedef struct {
    float speed; // velocity in km/h
    float dist; // obstacle distance in m
} sensorData;

// Sctructure for storing trigger data
typedef struct {
    bool actv_break; // brake activation status
} triggerData;

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

// Fuction for choose to take action 

int main()
{

    printf("Controller process PID: %d\n", getpid());

    int brake_pedal;
    int speed;

    mqd_t mq_receiver = create_mq(MQ_NAME);

    signal(SIGINT, close_mq);

    while(1)
    {
        read_mq(mq_receiver, &brake_pedal, &speed);
        printf("Brake pedal: %d, Speed: %d\n", brake_pedal, speed);
        sleep(1);
    }

    return 0;
}
