#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "constants.h"
#include "mq_utils.h"
#include "sensors_input.h"
#include <pthread.h>
#include <stdbool.h>
#include "dbc.h"

void* mainWorkingLoop(void *arg);
void translateAndCallCanMsg(can_msg captured_frame);
void updateInternalPedalsState(can_msg captured_frame);
void updateInternalSpeedState(can_msg captured_frame);
void updateInternalObstacleState(can_msg captured_frame);
void updateInternalCarCState(can_msg captured_frame);

pthread_t aeb_controller_id;
sensors_input_data aeb_internal_state = {
    .vehicle_velocity = 0.0, 
    .has_obstacle = false, 
    .obstacle_distance = 0.0, 
    .brake_pedal = false, 
    .accelerator_pedal = false, 
    .on_off_aeb_system = true
};

//can_msg captured_can_frame;
can_msg captured_can_frame = {
    .identifier = 0x0CFFB027,
    .dataFrame  = {0x08, 0x07, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}
};

int main(){
    int aeb_controller_thr;

    aeb_controller_thr = pthread_create(&aeb_controller_id, NULL, mainWorkingLoop, NULL);
    if(aeb_controller_thr != 0){
        perror("AEB Controller: it wasn't possible to create the associated thread\n");
        exit(53);
    }
    aeb_controller_thr = pthread_join(aeb_controller_id, NULL);

    return 0;
}

void* mainWorkingLoop(void *arg){ // Main Loop function for our AEB Controller ECU

    // Step 01: Get CAN frames from the message queue

    //read_mq(sensors_mq, &captured_can_frame);

    // Step 02: Translate the recieved CAN frame, according to its id
    // Step 03: Call the correct Function, based on the new data recieved
    translateAndCallCanMsg(captured_can_frame); 


    // Step 04: Reactions from the AEB System: based on the new data, what should the AEB controller do?

    // Testing changes, exclude this on production code
    printf("vehicle_velocity: %lf\n", aeb_internal_state.vehicle_velocity);
    printf("has_obstacle: %s\n", aeb_internal_state.has_obstacle ? "true" : "false");
    printf("obstacle_distance: %lf\n", aeb_internal_state.obstacle_distance);
    printf("brake_pedal: %s\n", aeb_internal_state.brake_pedal ? "true" : "false");
    printf("accelerator_pedal: %s\n", aeb_internal_state.accelerator_pedal ? "true" : "false");
    printf("on_off_aeb_system: %s\n", aeb_internal_state.on_off_aeb_system ? "true" : "false");

    // Step 05: Simple sleep timer? This will change when new functions to fulfill requirements are added

    printf("Placeholder\n");
    return NULL;
}

void translateAndCallCanMsg(can_msg captured_frame){
    switch(captured_can_frame.identifier){
        case ID_PEDALS:
            updateInternalPedalsState(captured_frame);
            break;
        case ID_SPEED_S:
            updateInternalSpeedState(captured_frame);
            break;
        case ID_OBSTACLE_S:
            updateInternalObstacleState(captured_frame);
            break;
        case ID_CAR_C:
            updateInternalCarCState(captured_frame);
            break;
        default:
            printf("CAN Identifier unkown\n");
            break;
    }
}

void updateInternalPedalsState(can_msg captured_frame){
    if(captured_frame.dataFrame[0] == 0x00){
        aeb_internal_state.accelerator_pedal = false;
    } else if(captured_frame.dataFrame[0] == 0x01){
        aeb_internal_state.accelerator_pedal = true;
    } else {
        ;
    }

    if(captured_frame.dataFrame[1] == 0x00){
        aeb_internal_state.brake_pedal = false;
    } else if(captured_frame.dataFrame[1] == 0x01){
        aeb_internal_state.brake_pedal = true;
    } else {
        ;
    }
}

void updateInternalSpeedState(can_msg captured_frame){
    unsigned int data_speed;
    double new_internal_speed;
    
    // THIS FUNCTION LACKS SPECIAL CONDITIONS AND MAXIMUM SPEED CHECKS

    // SPECIAL CONDITIONS GO HERE

    // Conversion from CAN data frame, according to dbc in the requirement file
    data_speed = captured_frame.dataFrame[0] + (captured_frame.dataFrame[1] << 8);
    new_internal_speed = data_speed * RES_SPEED_S;
    
    // MAXIMUM SPEED CHECKS GO HERE

    aeb_internal_state.vehicle_velocity = new_internal_speed;
}

void updateInternalObstacleState(can_msg captured_frame){
    unsigned int data_distance;
    double new_internal_distance;

    if(captured_frame.dataFrame[2] == 0x00){
        aeb_internal_state.has_obstacle = false;
    } else if(captured_frame.dataFrame[2] == 0x01){
        aeb_internal_state.has_obstacle = true;
    } else {
        ;
    }
    
    // THIS FUNCTION ALSO LACKS SPECIAL CONDITIONS AND MAXIMUM DISTANCE CHECKS
    // BOTH SHOULD BE BEFORE THE CONVERSION BELOW

    // Conversion from CAN data frame, according to dbc in the requirement file
    data_distance = captured_frame.dataFrame[0] + (captured_frame.dataFrame[1] << 8);
    new_internal_distance = data_distance * RES_OBSTACLE_S;


    aeb_internal_state.obstacle_distance = new_internal_distance;
}

void updateInternalCarCState(can_msg captured_frame){
    if(captured_frame.dataFrame[0] == 0x00){
        aeb_internal_state.on_off_aeb_system = false;
    } else if(captured_frame.dataFrame[0] == 0x01){
        aeb_internal_state.on_off_aeb_system = true;
    } else {
        ;
    }
}