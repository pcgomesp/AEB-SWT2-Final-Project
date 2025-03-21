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

// Definitions and constants
#define QUEUE_NAME "/my_queue"
#define MAX_SIZE 1024

// Structure for storing sensor data
typedef struct {
    float speed; // velocity in km/h
    float dist; // obstacle distance in m
} sensorData;

// Sctructure for storing trigger data
typedef struct {
    bool actv_break; // brake activation status
} triggerData;

// Function to create a message queue
mqd_t create_mq(char *mq_name) {
    mqd_t mq;
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MAX_SIZE;
    attr.mq_curmsgs = 0;

    mq = mq_open(mq_name, O_CREAT | O_RDWR, 0666, &attr);
    if (mq == (mqd_t) -1) {
        perror("mq_open");
        exit(1);
    }
    return mq;
}

// Function to close a message queue
void close_mq(mqd_t mq) {
    if (mq_close(mq) == -1) {
        perror("mq_close");
        exit(1);
    }
}

// Function to read from a message queue
void read_mq(mqd_t mq_receiver, int *brake_pedal, int *speed) {
    char buffer[MAX_SIZE];
    ssize_t bytes_read;

    bytes_read = mq_receive(mq_receiver, buffer, MAX_SIZE, NULL);
    if (bytes_read >= 0) {
        buffer[bytes_read] = '\0'; // Null terminate the string
        // Parse brake_pedal and speed from buffer if required
        // Example (assuming the data is passed as a string):
        sscanf(buffer, "%d,%d", brake_pedal, speed);
    } else {
        perror("mq_receive");
    }
}

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
