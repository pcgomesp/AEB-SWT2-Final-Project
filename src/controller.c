// Necessary libraries
#include <stdio.h>
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

    // Calculate the elapsed time since the last call
    clock_gettime(CLOCK_REALTIME, &current_time);
    elapsed_time = (double)(current_time.tv_sec - start_time.tv_sec) + (double)(current_time.tv_nsec - start_time.tv_nsec) / 1e9;
    start_time = current_time;

    if (prev_spd != 0.0) {
        accel = ((spd - prev_spd)*3.6) / elapsed_time; // acceleration in m/s^2
    }
    prev_spd = spd;

    return accel;
}

// Function for calculating the time to collision (TTC)
float ttc_calc(float dis_rel, float spd_rel) {
    // Quadratic equation coefficients - basic cinematics
    float a = accel_calc(spd_rel);
    float b = spd_rel;
    float c = dis_rel;
    float t1, t2;

    // Calculating the discriminant
    double delta = b * b - 2 * a * c;

    if (delta < 0) return -1.0; // No real roots

    else if (delta == 0) return -b / a;

    else {
        // calculating the root
        t1 = (-b + sqrt(delta)) / a;

        // returning the positive root
        return t1;
    }
}

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
