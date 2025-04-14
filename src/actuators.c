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


//Workaround to avoid the main function in this file
// Write just the main function in the test file
//Put the #ifndef TEST_MODE in the test file, as bellow
//After the last line of main, put #endif

//The next step is put the flag TEST_MODE in the Makefile
//See the example:
// 	$(CC) $(CFLAGS) -DTEST_MODE test/test_actuators.c src/actuators.c test/unity.c -o test/test_actuators -Iinc -Itest -lpthread
//Put the flag TEST_MODE in the Makefile: -DTEST_MODE
#ifndef TEST_MODE 
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
#endif 


/**
 * @brief Main loop for processing actuator messages.
 *
 * This function runs in a separate thread and continuously reads messages from the actuators message queue.
 * It processes each message to update the internal state of the actuators and logs the event.
 * If the message queue is empty for a specified number of iterations (`LOOP_EMPTY_ITERATIONS_MAX`), 
 * the loop exits, signaling that no more messages are expected.
 *
 * @param arg Unused parameter (can be NULL).
 * @return NULL
 *
 * @details
 * - Reads messages from the `actuators_mq` message queue using `read_mq`.
 * - If a message is successfully read, it resets the `empty_mq_counter` and processes the message using `actuatorsTranslateCanMsg`.
 * - If the queue is empty, increments the `empty_mq_counter`.
 * - Logs each processed message using `log_event`, including the current state of the actuators.
 * - Waits for 200 milliseconds between iterations using `usleep`.
 * - Exits the loop and prints a message when the `empty_mq_counter` reaches `LOOP_EMPTY_ITERATIONS_MAX`.
 *
 * Implements [SwR-4](@ref SwR-4)
 */
void *actuatorsResponseLoop(void *arg)
{
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

        uint32_t event_id = captured_can_frame.identifier; 

        log_event("AEB1", event_id, actuators_state); // [SwR-4]


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
        //printf("Actuators: Empty message received\n");
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