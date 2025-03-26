// Necessary libraries
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "constants.h"
#include "mq_utils.h"
#include "sensors_input.h"
#include <pthread.h>
#include <stdbool.h>
#include "dbc.h"
#include "file_reader.h"

void* getSensorsData(void *arg);
can_msg conv2CANCarClusterData(bool on_off_aeb_system);
can_msg conv2CANVelocityData(bool vehicle_direction, double vehicle_velocity);
can_msg conv2CANObstacleData(bool has_obstacle, double obstacle_distance);
can_msg conv2CANPedalsData(bool brake_pedal, bool accelerator_pedal);

mqd_t sensors_mq;
pthread_t sensors_id;
sensors_input_data sensorsData;

can_msg can_car_cluster, can_velocity_sensor, can_obstacle_sensor, can_pedals_sensor;

int main(){
    int sensors_thr;

    sensors_mq = create_mq(SENSORS_MQ);
    // sensors_mq = open_mq(SENSORS_MQ);

    const char *filename = "cts/cenario.txt";
    FILE *file = open_file(filename); // uses the modularized function to open the file

    sensors_thr = pthread_create(&sensors_id, NULL, getSensorsData, file); //Changed the argument from null to file(the last argument)
    if(sensors_thr != 0){
        perror("Sensors: it wasn't possible to create the associated thread\n");
        exit(52);
    }
    sensors_thr = pthread_join(sensors_id, NULL);
    
    // close_mq(sensors_mq, SENSORS_MQ);

    return 0;
}

void* getSensorsData(void *arg){
    // Part 1: take data from txt/csv file
    // Part 2: Convert data to can_msg format
    // Part 3: Send all four frames to sensors_mq    
    // Part 4: simple sleep timer;

    FILE *file = (FILE*)arg; // Recebe o arquivo como argumento

    while (1) {
        // Read a new line from the file
        if (read_sensor_data(file, &sensorsData)) {
            can_car_cluster     = conv2CANCarClusterData(sensorsData.on_off_aeb_system);
            can_velocity_sensor = conv2CANVelocityData(sensorsData.reverseEnabled, sensorsData.vehicle_velocity);
            can_obstacle_sensor = conv2CANObstacleData(sensorsData.has_obstacle, sensorsData.obstacle_distance);
            can_pedals_sensor   = conv2CANPedalsData(sensorsData.brake_pedal, sensorsData.accelerator_pedal);

            write_mq(sensors_mq, &can_car_cluster);
            write_mq(sensors_mq, &can_velocity_sensor);
            write_mq(sensors_mq, &can_obstacle_sensor);
            write_mq(sensors_mq, &can_pedals_sensor);

            // PlaceHolder: just some simple prints to see if the data is being converted in the right way
            //print_can_msg(&can_car_cluster);
            //print_can_msg(&can_velocity_sensor);
            //print_can_msg(&can_obstacle_sensor);
            //print_can_msg(&can_pedals_sensor);
        
            printf("New line.\n"); //This line is used for see the break of line
        } else {
            // If a new line can't be read, the end of the file was reached
            printf("EOF reached.\n");
            break;
        }

        sleep(1); // Wait for 1 second before reading the next line
    }
        
    //printf("Placeholder\n");
    fclose(file); // Closes the file at the end
    return NULL;
}

// The location of information in the data frame location, in the following functions, 
// is according to the dbc file in the requirements specification

can_msg conv2CANCarClusterData(bool on_off_aeb_system){
    can_msg aux = {.identifier = ID_CAR_C, .dataFrame = BASE_DATA_FRAME};

    // Enable or disable AEB data encapsulation
    if(on_off_aeb_system){
        aux.dataFrame[0] = 0x01;
    }
    else{
        aux.dataFrame[0] = 0x00;
    }

    return aux;
}

can_msg conv2CANVelocityData(bool vehicle_direction, double vehicle_velocity){
    can_msg aux = {.identifier = ID_SPEED_S, .dataFrame = BASE_DATA_FRAME};

    // Vehicle direction (forward or reverse) data encapsulation
    if(vehicle_direction){
        aux.dataFrame[2] = 0x01;
    } else {
        aux.dataFrame[2] = 0x00;
    }

    // Speed data ​​encapsulation
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

    // Obstacle detection data encapsulation
    if(has_obstacle){
        aux.dataFrame[2] = 0x01;
    } else {
        aux.dataFrame[2] = 0x00;
    }

    // Obstacle distance data ​​encapsulation
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

    // Brake pedal activation data encapsulation
    if(brake_pedal){
        aux.dataFrame[1] = 0x01;
    } else {
        aux.dataFrame[1] = 0x00;
    }

    // Accelerator pedal activation data encapsulation
    if(accelerator_pedal){
        aux.dataFrame[0] = 0x01;
    } else {
        aux.dataFrame[0] = 0x00;
    }

    return aux;
}