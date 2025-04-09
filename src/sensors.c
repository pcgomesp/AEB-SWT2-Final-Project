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

void *getSensorsData(void *arg);
can_msg conv2CANCarClusterData(bool on_off_aeb_system);
can_msg conv2CANVelocityData(bool vehicle_direction, double relative_velocity, double relative_acceleration);
can_msg conv2CANObstacleData(bool has_obstacle, double obstacle_distance);
can_msg conv2CANPedalsData(bool brake_pedal, bool accelerator_pedal);

mqd_t sensors_mq;
pthread_t sensors_id;
sensors_input_data sensorsData;

can_msg can_car_cluster, can_velocity_sensor, can_obstacle_sensor, can_pedals_sensor;

#ifndef TEST_MODE 
int main()
{
    int sensors_thr;

    sensors_mq = create_mq(SENSORS_MQ);

    const char *filename = "cts/cenario.txt";
    FILE *file = open_file(filename); // uses the modularized function to open the file

    sensors_thr = pthread_create(&sensors_id, NULL, getSensorsData, file); // Changed the argument from null to file(the last argument)
    if (sensors_thr != 0)
    {
        perror("Sensors: it wasn't possible to create the associated thread\n");
        exit(52);
    }
    sensors_thr = pthread_join(sensors_id, NULL);

    return 0;
}

/**
 * @brief Function that encapsulates data from a file into CAN frames and sends it to the message queue.
 * 
 * This function is runned by the thread sensors_thr. It calls the other functions of the program
 * to read data from the file, encode it into CAN frames and send it to sensors message queue. 
 * 
 * @param arg Arguments passed to the thread (in this case it is the file pointer).
 * @return NULL.
 * 
*/
void* getSensorsData(void *arg)
{
    FILE *file = (FILE *) arg;
    while (1)
    {
        // Read a new line from the file [SwR-9]
        if (read_sensor_data(file, &sensorsData))
        {
            can_car_cluster = conv2CANCarClusterData(sensorsData.on_off_aeb_system);
            can_velocity_sensor = conv2CANVelocityData(sensorsData.reverseEnabled, sensorsData.relative_velocity, sensorsData.relative_acceleration); // [SwR-10]
            can_obstacle_sensor = conv2CANObstacleData(sensorsData.has_obstacle, sensorsData.obstacle_distance);
            can_pedals_sensor = conv2CANPedalsData(sensorsData.brake_pedal, sensorsData.accelerator_pedal);

            write_mq(sensors_mq, &can_car_cluster);
            write_mq(sensors_mq, &can_velocity_sensor);
            write_mq(sensors_mq, &can_obstacle_sensor);
            write_mq(sensors_mq, &can_pedals_sensor);

            //printf("New line.\n"); // This line is used for see the break of line
        }
        else
        {
            // If a new line can't be read, the end of the file was reached
            printf("EOF reached.\n");
            break;
        }

        sleep(1); // Wait for 1 second before reading the next line
    }

    fclose(file);
    return NULL;
}
#endif

// The location of information in the data frame location, in the following functions,
// is according to the dbc file in the requirements specification

/**
 * @brief Function that encapsulates data into the Car Cluster CAN frame.
 * 
 * @param on_off_aeb_system Argument that refers the power state of the AEB System (ON or OFF).
 * 
 * @return structure that contains the CAN message ID and Frame.
 * 
*/
can_msg conv2CANCarClusterData(bool on_off_aeb_system)
{
    can_msg aux = {.identifier = ID_CAR_C, .dataFrame = BASE_DATA_FRAME};

    // Enable or disable AEB data encapsulation
    if (on_off_aeb_system)
    {
        aux.dataFrame[0] = 0x01;
    }
    else
    {
        aux.dataFrame[0] = 0x00;
    }

    return aux;
}


/**
 * @brief Function that encapsulates data into the Speed CAN frame.
 * 
 * This function receives data of the vehicle direction, relative velocity and 
 * relative acceleration and encapsulates it into the Speed CAN frame.
 * 
 * @param vehicle_direction Vehicle direction (forward or reverse).
 * @param relative_velocity Relative velocity of the vehicle.
 * @param relative_acceleration Relative acceleration of the vehicle.
 * 
 * @return structure that contains the CAN message ID and Frame.
 * 
*/
can_msg conv2CANVelocityData(bool vehicle_direction, double relative_velocity, double relative_acceleration)
{
    //printf("Rel acel: %lf\n", relative_acceleration);
    can_msg aux = {.identifier = ID_SPEED_S, .dataFrame = BASE_DATA_FRAME};

    // Vehicle direction (forward or reverse) data encapsulation
    if (vehicle_direction)
    {
        aux.dataFrame[2] = 0x01;
    }
    else
    {
        aux.dataFrame[2] = 0x00;
    }

    // Speed data ​​encapsulation
    unsigned int data_speed = relative_velocity / RES_SPEED_S;
    unsigned char ms_speed, ls_speed;
    ls_speed = data_speed;
    ms_speed = data_speed >> 8;

    // Defines most and least significant bytes, according to the DBC specification
    aux.dataFrame[0] = ls_speed;
    aux.dataFrame[1] = ms_speed;

    // Acceleration data ​​encapsulation
    double aux_acel = relative_acceleration;
    if(aux_acel < 0){
        aux_acel *= -1;
        aux.dataFrame[5] = 0x01;
    } else {
        aux.dataFrame[5] = 0x00;
    }

    unsigned int data_acel = ((aux_acel * RES_ACCELERATION_DIV_S) - OFFSET_ACCELERATION_S);
    unsigned char ms_acel, ls_acel;
    ls_acel = data_acel;
    ms_acel = data_acel >> 8;

    // Defines most and least significant bytes, according to the DBC specification
    aux.dataFrame[3] = ls_acel;
    aux.dataFrame[4] = ms_acel;

    return aux;
}


/**
 * @brief Function that encapsulates data into the Obstacle CAN frame.
 * 
 * This function encapsulates the data of the presence of an obstacle and it's distance
 * to the vehicle into the Obstacle CAN frame.
 * 
 * @param has_obstacle Argument that refers if an obstacle is present or not.
 * @param obstacle_distance Distance from the vehicle to the obstacle.
 * 
 * @return structure that contains the CAN message ID and Frame.
 * 
*/
can_msg conv2CANObstacleData(bool has_obstacle, double obstacle_distance)
{
    can_msg aux = {.identifier = ID_OBSTACLE_S, .dataFrame = BASE_DATA_FRAME};

    // Obstacle detection data encapsulation
    if (has_obstacle)
    {
        aux.dataFrame[2] = 0x01;
    }
    else
    {
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


/**
 * @brief Function that encapsulates data into the Pedals CAN frame.
 * 
 * This function encapsulates the data of the brake and accelerator pedals into 
 * the Pedals CAN frame.
 * 
 * @param brake_pedal Argument that refers if the brake is pressed or not.
 * @param accelerator_pedal Argument that refers if the accelerator is pressed or not.
 * 
 * @return structure that contains the CAN message ID and Frame.
 * 
*/
can_msg conv2CANPedalsData(bool brake_pedal, bool accelerator_pedal)
{
    can_msg aux = {.identifier = ID_PEDALS, .dataFrame = BASE_DATA_FRAME};

    // Brake pedal activation data encapsulation
    if (brake_pedal)
    {
        aux.dataFrame[1] = 0x01;
    }
    else
    {
        aux.dataFrame[1] = 0x00;
    }

    // Accelerator pedal activation data encapsulation
    if (accelerator_pedal)
    {
        aux.dataFrame[0] = 0x01;
    }
    else
    {
        aux.dataFrame[0] = 0x00;
    }

    return aux;
}