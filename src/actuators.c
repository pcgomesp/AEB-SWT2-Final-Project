#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mqueue.h>
#include <stdbool.h>
#include <pthread.h>
#include "mq_utils.h"
#include "constants.h"
#include "actuators.h"
#include "dbc.h"
#include "log_utils.h"

#define LOOP_EMPTY_ITERATIONS_MAX 11

void *actuatorsResponseLoop(void *arg);
void actuatorsTranslateCanMsg(can_msg captured_frame);
void updateInternalActuatorsState(can_msg captured_frame);

mqd_t actuators_mq;
pthread_t actuators_id;

actuators_abstraction actuators_state = {
    .belt_tightness = false,
    .door_lock = true,
    .should_activate_abs = false,
    .alarm_led = false,
    .alarm_buzzer = true};

can_msg captured_can_frame = {
    .identifier = 0x0CFFB027,
    .dataFrame = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}};

int main()
{
    actuators_mq = open_mq(ACTUATORS_MQ);

    int actuators_thread;
    actuators_thread = pthread_create(&actuators_id, NULL, actuatorsResponseLoop, NULL);
    if (actuators_thread != 0)
    {
        perror("Actuators: it wasn't possible to create the associated thread\n");
        exit(54);
    }
    actuators_thread = pthread_join(actuators_id, NULL);

    return 0;
}

void *actuatorsResponseLoop(void *arg)
{
    // Step 01: Recieve message from Message Queue, with new data sent by AEB
    // Step 02: Convert data from the AEB can_msg to actuators_state memory
    // Step 03: Do the right activation from the actuator ->
    // i.e., in our project, writing the correct expected output in a txt ou csv, since this is an abstraction
    // Step 04: sleep, waiting the next message -> loop
    // we must define a Stop criteria btw

    int empty_mq_counter = 0;
    while (empty_mq_counter < LOOP_EMPTY_ITERATIONS_MAX)
    {
        if (read_mq(actuators_mq, &captured_can_frame) != -1)
        {
            empty_mq_counter = 0;
            actuatorsTranslateCanMsg(captured_can_frame);
        }
        else
        {
            empty_mq_counter++;
        }

        uint32_t event_id = 0x63A5D2E1; // A simple event_id for testing purposes

        // write in file here
        // The condition below is for test porpuse, should be changed to a ttc value
        log_event("AEB1", event_id, actuators_state); // [SwR-4]

        printf("belt_tightness: %s\n", actuators_state.belt_tightness ? "true" : "false");
        printf("door_lock: %s\n", actuators_state.door_lock ? "true" : "false");
        printf("should_activate_abs: %s\n", actuators_state.should_activate_abs ? "true" : "false");
        printf("alarm_led: %s\n", actuators_state.alarm_led ? "true" : "false");
        printf("alarm_buzzer: %s\n", actuators_state.alarm_buzzer ? "true" : "false");

        usleep(200000); // Deprected, change for function other later
    }

    printf("Actuators: empty_mq_counter reached the limit, exiting\n");
    return NULL;
}

void actuatorsTranslateCanMsg(can_msg captured_frame)
{
    switch (captured_frame.identifier)
    {
    case ID_AEB_S:
        updateInternalActuatorsState(captured_frame);
        break;
    case ID_EMPTY:
        printf("Actuators: Empty message received\n");
        break;
    default:
        printf("Actuators: CAN Identifier unknown\n");
        break;
    }
}

void updateInternalActuatorsState(can_msg captured_frame)
{
    if (captured_frame.dataFrame[1] == 0x01)
    {
        actuators_state.belt_tightness = true;
        actuators_state.door_lock = false;
        actuators_state.should_activate_abs = true;
        actuators_state.alarm_led = true;
        actuators_state.alarm_buzzer = true;
    }
    else if (captured_frame.dataFrame[0] == 0x01)
    {
        actuators_state.belt_tightness = false;
        actuators_state.door_lock = true;
        actuators_state.should_activate_abs = false;
        actuators_state.alarm_led = true;
        actuators_state.alarm_buzzer = true;
    }
    else
    {
        actuators_state.belt_tightness = false;
        actuators_state.door_lock = true;
        actuators_state.should_activate_abs = false;
        actuators_state.alarm_led = false;
        actuators_state.alarm_buzzer = false;
    }
}