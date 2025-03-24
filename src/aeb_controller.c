#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "constants.h"
#include "mq_utils.h"
#include "sensors_input.h"
#include <pthread.h>
#include <stdbool.h>
#include "dbc.h"
#include "actuators.h"
#include <time.h>

#define LOOP_EMPTY_ITERATIONS_MAX 11

void* mainWorkingLoop(void *arg);
void translateAndCallCanMsg(can_msg captured_frame);
void updateInternalPedalsState(can_msg captured_frame);
void updateInternalSpeedState(can_msg captured_frame);
void updateInternalObstacleState(can_msg captured_frame);
void updateInternalCarCState(can_msg captured_frame);
can_msg updateCanMsgOutput(double ref_ttc);

mqd_t sensors_mq, actuators_mq;
pthread_t aeb_controller_id;

sensors_input_data aeb_internal_state = {
    .vehicle_velocity = 0.0, 
    .has_obstacle = false, 
    .obstacle_distance = 0.0, 
    .brake_pedal = false, 
    .accelerator_pedal = false, 
    .on_off_aeb_system = true
};
// actuators_abstraction send_actuators_state = {
//     .belt_tightness = false,
//     .door_lock = true,
//     .should_activate_abs = false,
//     .alarm_led = true,
//     .alarm_buzzer = true
// };


can_msg captured_can_frame = {
    .identifier = 0x0CFFB027,
    .dataFrame  = {0x08, 0x07, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}
};
can_msg out_can_frame = {
    .identifier = 0x18FFA027,
    .dataFrame  = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}
};

int main(){
    int aeb_controller_thr;

    sensors_mq = open_mq(SENSORS_MQ);
    actuators_mq = create_mq(ACTUATORS_MQ);

    aeb_controller_thr = pthread_create(&aeb_controller_id, NULL, mainWorkingLoop, NULL);
    if(aeb_controller_thr != 0){
        perror("AEB Controller: it wasn't possible to create the associated thread\n");
        exit(53);
    }
    aeb_controller_thr = pthread_join(aeb_controller_id, NULL);

    close_mq(actuators_mq, ACTUATORS_MQ);

    return 0;
}

void* mainWorkingLoop(void *arg){ // Main Loop function for our AEB Controller ECU
    // Step 01: Get CAN frames from the message queue
    // Step 02: Translate the recieved CAN frame, according to its id
    // Step 03: Call the correct Function, based on the new data recieved
    // Step 04: Reactions from the AEB System: based on the new data, what should the AEB controller do?
        // 4.1 - Calculate new ttc
        // 4.2 - Update send_actuators_state
        // Should we separate more or can we let everything in this single thread?
        // Will separating things make us need to worry more about synchronization?
    // Step 05: Simple sleep timer? This will change when new functions to fulfill requirements are added

    int no_message_counter = 0;
    int result;

    // i don't know why yet, but this condition doesn't make it get out of the while/thread. Suggestions are accepted!
    while(no_message_counter < LOOP_EMPTY_ITERATIONS_MAX){
        result = read_mq(sensors_mq, &captured_can_frame);
        if(result == 0){
            translateAndCallCanMsg(captured_can_frame); 
            no_message_counter = 0;
        } else if (result == -1){
            no_message_counter++;
        } else {
            perror("\n");
            break;
        }
        
        // calculate ttc here?
        out_can_frame = updateCanMsgOutput(2.1);
        write_mq(actuators_mq, &out_can_frame);

        // Testing changes, exclude this on production code
        printf("vehicle_velocity: %lf\n", aeb_internal_state.vehicle_velocity);
        printf("has_obstacle: %s\n", aeb_internal_state.has_obstacle ? "true" : "false");
        printf("obstacle_distance: %lf\n", aeb_internal_state.obstacle_distance);
        printf("brake_pedal: %s\n", aeb_internal_state.brake_pedal ? "true" : "false");
        printf("accelerator_pedal: %s\n", aeb_internal_state.accelerator_pedal ? "true" : "false");
        printf("on_off_aeb_system: %s\n", aeb_internal_state.on_off_aeb_system ? "true" : "false");
    
        usleep(500000); // Deprected, change for function other later
    }

    printf("Placeholder\n");
    return NULL;
}

void translateAndCallCanMsg(can_msg captured_frame){
    switch(captured_frame.identifier){
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

can_msg updateCanMsgOutput(double ref_ttc){
    can_msg aux = {.identifier = ID_AEB_S, .dataFrame = BASE_DATA_FRAME};

    if(0 < ref_ttc && ref_ttc < 1){
        aux.dataFrame[0] = 0x01;
        aux.dataFrame[1] = 0x01;
    } else if (0 < ref_ttc && ref_ttc < 2){
        aux.dataFrame[0] = 0x01;
        aux.dataFrame[1] = 0x00;
    } else {
        aux.dataFrame[0] = 0x00;
        aux.dataFrame[1] = 0x00;
    }

    return aux;
}