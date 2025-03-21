#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "constants.h"
#include "mq_utils.h"
#include "sensors_input.h"
#include <pthread.h>
#include <stdbool.h>
#include "dbc.h"

void* getSensorsData(void *arg);
can_msg conv2CANCarClusterData(bool on_off_aeb_system);
can_msg conv2CANVelocityData(double vehicle_velocity);
can_msg conv2CANObstacleData(bool has_obstacle, double obstacle_distance);
can_msg conv2CANPedalsData(bool brake_pedal, bool accelerator_pedal);

pthread_t sensors_id;
char *shm_ptr;
sensors_input_data sensorsData = {
    .vehicle_velocity = 50.0, 
    .has_obstacle = false, 
    .obstacle_distance = 90.0, 
    .brake_pedal = false, 
    .accelerator_pedal = false, 
    .on_off_aeb_system = true
};

can_msg can_car_cluster, can_velocity_sensor, can_obstacle_sensor, can_pedals_sensor;

int main(){
    int sensors_thr;
    // Falta o SHM_LOCATION
    // int shm_fd = shm_open(SHM_LOCATION, O_RDWR , 0666); // Acess to shared memory, not yet implemented
    // shm_ptr = (char *)mmap(0, BUFFER_SIZE_SHM, PROT_WRITE, MAP_SHARED, shm_fd, 0); // Shared mem mapping to program internal mem
    
    // FALTA O SMPH_NAME
    // smph = sem_open(SMPH_NAME, 0); // Acess to Posix Semaphore create on main.c
    // if (smph == SEM_FAILED) {
    //     perror("Sensors: it wasn't possible to acess the semaphore");
    //     exit(51);
    // }

    //mqd_t mq_sender = open_mq(SENSORS_MQ);

    sensors_thr = pthread_create(&sensors_id, NULL, getSensorsData, NULL);
    if(sensors_thr != 0){
        perror("Sensors: it wasn't possible to create the associated thread\n");
        exit(52);
    }
    sensors_thr = pthread_join(sensors_id, NULL);
    //close(shm_fd);

    return 0;
}

void* getSensorsData(void *arg){
    // Part 1: take data from shared memory

    // while(1){
    //     // MUST HAVE a Semaphore here

    //     // Take data from the shared memory to sensorsData struct variable
    //     // Convert to can_msg format
    //     // Pass all the data, writing 4 different can_msg messages 
        
    //     sleep(1);
    // }

    // Part 2: Convert data to can_msg format
    can_car_cluster     = conv2CANCarClusterData(sensorsData.on_off_aeb_system);
    can_velocity_sensor = conv2CANVelocityData(sensorsData.vehicle_velocity);
    can_obstacle_sensor = conv2CANObstacleData(sensorsData.has_obstacle, sensorsData.obstacle_distance);
    can_pedals_sensor   = conv2CANPedalsData(sensorsData.brake_pedal, sensorsData.accelerator_pedal);

    // Part 3: Send all four frames to sensors_mq
    
    
    
    // Part 4: simple sleep timer here;
    

    // PlaceHolder: just some simple prints to see if the data is being converted in the right way
    print_can_msg(&can_car_cluster);
    print_can_msg(&can_velocity_sensor);
    print_can_msg(&can_obstacle_sensor);
    print_can_msg(&can_pedals_sensor);

    printf("Placeholder\n");
    return NULL;
}

// The location of information in the data frame location, in the following functions, 
// is according to the dbc file in the requirements specification

can_msg conv2CANCarClusterData(bool on_off_aeb_system){
    can_msg aux = {.identifier = ID_CAR_C, .dataFrame = BASE_DATA_FRAME};

    if(on_off_aeb_system){
        aux.dataFrame[0] = 0x01;
    }
    else{
        aux.dataFrame[0] = 0x00;
    }

    return aux;
}

can_msg conv2CANVelocityData(double vehicle_velocity){
    can_msg aux = {.identifier = ID_SPEED_S, .dataFrame = BASE_DATA_FRAME};

    // Speed data ​​encapsulation: Data_speed = speed/res - offset => speed * 256
    unsigned int data_speed = vehicle_velocity / RES_SPEED_S;
    unsigned char ms_speed, ls_speed;
    ls_speed = data_speed;
    ms_speed = data_speed >> 8;

    // Defines most and least significant bytes, according to the DBC specification
    aux.dataFrame[0] = ls_speed;
    aux.dataFrame[1] = ms_speed;

    return aux;
}

can_msg conv2CANObstacleData(bool has_obstacle, double obstacle_distance){
    can_msg aux = {.identifier = ID_OBSTACLE_S, .dataFrame = BASE_DATA_FRAME};

    if(has_obstacle){
        aux.dataFrame[2] = 0x01;
    } else {
        aux.dataFrame[2] = 0x00;
    }

    // Obstacle distance data ​​encapsulation: Data_speed = speed/res - offset => speed * 256
    unsigned int data_distance = obstacle_distance / RES_OBSTACLE_S;
    unsigned char ms_distance, ls_distance;
    ls_distance = data_distance;
    ms_distance = data_distance >> 8;

    // Defines most and least significant bytes, according to the DBC specification
    aux.dataFrame[0] = ls_distance;
    aux.dataFrame[1] = ms_distance;

    return aux;
}

can_msg conv2CANPedalsData(bool brake_pedal, bool accelerator_pedal){
    can_msg aux = {.identifier = ID_PEDALS, .dataFrame = BASE_DATA_FRAME};

    if(brake_pedal){
        aux.dataFrame[1] = 0x01;
    } else {
        aux.dataFrame[1] = 0x00;
    }

    if(accelerator_pedal){
        aux.dataFrame[0] = 0x01;
    } else {
        aux.dataFrame[0] = 0x00;
    }

    return aux;
}