/**
 * @file controller.c
 * @brief Controller module responsible for managing actuator logic through message queue handling.
 * 
 * This module handles the reception and processing of CAN messages related to actuators via POSIX
 * message queues. It spawns a separate thread to continuously read incoming messages, update the 
 * internal state of the actuators accordingly, and log each event for traceability and diagnostics.
 */

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
 * 
 * \anchor actuatorsResponseLoop
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

/**
 * @brief Translates a CAN message into actuator commands.
 *
 * This function processes a captured CAN message and determines the appropriate action
 * based on the message's identifier. It updates the actuators' internal state or logs
 * a warning if the message identifier is unknown.
 *
 * @param captured_frame The captured CAN message to be processed.
 * 
 * \anchor actuatorsTranslateCanMsg
 *
 * @details
 * - If the identifier is `ID_AEB_S`, the function calls `updateInternalActuatorsState` to update
 *   the actuators' internal state based on the message's data.
 * - If the identifier is `ID_EMPTY`, the function does nothing, as it represents an empty message.
 * - For any other identifier, the function logs a warning indicating that the identifier is unknown.
 *
 * @note This function is a key part of the actuators module, as it ensures that the actuators' state
 * remains synchronized with the incoming CAN messages.
 *
 * @see updateInternalActuatorsState
 */
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

/**
 * @brief Updates the internal state of the actuators based on a CAN message.
 *
 * This function processes the data frame of a captured CAN message and updates
 * the internal state of the actuators accordingly. It determines the state of
 * various actuators, such as belt tightness, door lock, ABS activation, alarm LED,
 * and alarm buzzer, based on specific conditions in the message's data frame.
 *
 * @param captured_frame The captured CAN message containing the data to update the actuators' state.
 *
 * @details
 * - If `dataFrame[1] == 0x01`, the function sets the actuators to an active state, enabling
 *   features like belt tightness, ABS activation, and alarms.
 * - If `dataFrame[0] == 0x01`, the function sets the actuators to a partially active state,
 *   enabling some features while disabling others.
 * - For any other values in the data frame, the function sets the actuators to a default
 *   inactive state, disabling all features except the door lock.
 *
 * @note This function is critical for ensuring that the actuators' state reflects the
 *       commands received via CAN messages. It is called by `actuatorsTranslateCanMsg`
 *       when a valid message with the identifier `ID_AEB_S` is received.
 * 
* \anchor updateInternalActuatorsState

 *
 * @see actuatorsTranslateCanMsg
 */
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