#include <stdio.h>
#include <unistd.h>
#include <mqueue.h>
#include "mq_utils.h"
#include "constants.h"
#include "actuators.h"
#include "dbc.h"

void* actuatorsResponseLoop(void *arg);

mqd_t actuators_mq;
pthread_t actuators_id;
actuators_abstraction actuators_state = {
    .belt_tightness = false,
    .door_lock = true,
    .should_activate_abs = false,
    .alarm_led = true,
    .alarm_buzzer = true
};

can_msg captured_can_frame = {
    .identifier = 0x0CFFB027,
    .dataFrame  = {0x08, 0x07, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}
};

int main(){
    //mqd_t mq_receiver = open_mq(ACTUATORS_MQ);
    // char buffer[MQ_MAX_MSG_SIZE];
    // while (1)
    // {
        //     read_mq(mq_receiver, buffer);
        //     printf("Received message: <%s>\n", buffer);
        //     sleep(1);
        // }    
    int actuators_thr;

    actuators_thr = pthread_create(&actuators_id, NULL, actuatorsResponseLoop, NULL);
    if(actuators_thr != 0){
        perror("Actuators: it wasn't possible to create the associated thread\n");
        exit(54);
    }
    actuators_thr = pthread_join(actuators_id, NULL);

    return 0;
}

void* actuatorsResponseLoop(void *arg){
    // Step 01: Recieve message from Message Queue, with new data sent by AEB

    // Step 02: Convert data from the AEB can_msg to actuators_state memory

    // Step 03: Do the right activation from the actuator -> 
    // i.e., in our project, writing the correct expected output in a txt ou csv, since this is an abstraction

    // Step 04: sleep, waiting the next message -> loop
    // we must define a Stop criteria btw

    return NULL;
}