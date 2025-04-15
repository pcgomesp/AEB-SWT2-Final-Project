// Necessary libraries
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <time.h>
#include "constants.h"
#include "mq_utils.h"
#include "sensors_input.h"
#include "dbc.h"
#include "actuators.h"
#include "ttc_control.h"

#define LOOP_EMPTY_ITERATIONS_MAX 11

/**
 * @enum aeb_controller_state
 * @brief Enum defining the possible states of the AEB system.
 */
typedef enum // Abstraction according to [SwR-12]
{
    AEB_STATE_ACTIVE,   /**< AEB system is active and performing actions */
    AEB_STATE_ALARM,    /**< AEB system is in alarm state, but not yet braking */
    AEB_STATE_BRAKE,    /**< AEB system is actively braking the vehicle */
    AEB_STATE_STANDBY   /**< AEB system is in standby, waiting for data or conditions */
} aeb_controller_state;

// Function prototypes
void *mainWorkingLoop(void *arg);
void print_info();
void translateAndCallCanMsg(can_msg captured_frame);
void updateInternalPedalsState(can_msg captured_frame);
void updateInternalSpeedState(can_msg captured_frame);
void updateInternalObstacleState(can_msg captured_frame);
void updateInternalCarCState(can_msg captured_frame);
can_msg updateCanMsgOutput(aeb_controller_state state);
aeb_controller_state getAEBState(sensors_input_data aeb_internal_state, double ttc);

// Global variables for message queues and internal state
mqd_t sensors_mq, actuators_mq; /**< Message queues for sensors and actuators */
pthread_t aeb_controller_id;     /**< Thread ID for the AEB controller */

sensors_input_data aeb_internal_state = {
    .relative_velocity = 0.0,
    .has_obstacle = false,
    .obstacle_distance = 0.0,
    .brake_pedal = false,
    .accelerator_pedal = false,
    .on_off_aeb_system = true,
    .reverseEnabled = false,
    .relative_acceleration = 0.0};

can_msg captured_can_frame = {
    .identifier = ID_CAR_C,
    .dataFrame = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}};

can_msg out_can_frame = {
    .identifier = ID_AEB_S,
    .dataFrame = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}};

can_msg empty_msg = { // [SwR-5]
    .identifier = ID_EMPTY,
    .dataFrame = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};

/**
 * @brief The main entry point of the AEB (Autonomous Emergency Braking) controller system.
 * 
 * This function initializes message queues for sensors and actuators, then starts the 
 * AEB controller thread to begin processing. It is designed for use in a production environment 
 * where the AEB controller operates continuously.
 * 
 * @return 0 on successful execution. If an error occurs while creating the AEB controller thread,
 *         the function will terminate with an error code.
 * 
 * @note This function is only included in production builds, as it is enclosed in 
 *       a preprocessor check (`#ifndef TEST_MODE_CONTROLLER`).
 */
#ifndef TEST_MODE // Main for the AEB controller process in production
 int main()
{
    // Open message queues for communication with sensors and actuators
    sensors_mq = open_mq(SENSORS_MQ);
    actuators_mq = open_mq(ACTUATORS_MQ);

    // Create the AEB controller thread
    int controller_thread = pthread_create(&aeb_controller_id, NULL, mainWorkingLoop, NULL);
    if (controller_thread != 0)
    {
        // If thread creation fails, print error and exit
        perror("AEB Controller: It wasn't possible to create the associated thread\n");
        exit(53);
    }

    // Wait for the controller thread to finish
    controller_thread = pthread_join(aeb_controller_id, NULL);

    return 0;
}

/**
 * @brief Main loop for the AEB controller that processes sensor data and makes decisions.
 * 
 * This function continuously checks the message queue for new sensor data, processes it, 
 * and sends commands to the actuators based on the calculated AEB state.
 * 
 * @param arg Arguments passed to the thread (not used here).
 * @return NULL.
 */
void *mainWorkingLoop(void *arg)
{
    aeb_controller_state state = AEB_STATE_STANDBY;

    int empty_mq_counter = 0;
    while (empty_mq_counter < LOOP_EMPTY_ITERATIONS_MAX)
    {
        if (read_mq(sensors_mq, &captured_can_frame) != -1) // Reads message from sensors [SwR-9]
        {
            empty_mq_counter = 0; // Reset counter if data is received

            translateAndCallCanMsg(captured_can_frame); // Process the received CAN message

            double ttc = ttc_calc(aeb_internal_state.obstacle_distance, aeb_internal_state.relative_velocity, 
                aeb_internal_state.relative_acceleration);

            state = getAEBState(aeb_internal_state, ttc);

            out_can_frame = updateCanMsgOutput(state);

            if (state == AEB_STATE_STANDBY) // [SwR-5]
                write_mq(actuators_mq, &empty_msg);  // Send empty message when in standby state
            else
                write_mq(actuators_mq, &out_can_frame);  // Send the appropriate message based on the current state

        }
        else
            empty_mq_counter++;  // Increment counter if no message is received

        usleep(200000);  // Wait for a short period before the next iteration (to be replaced later)
    }

    printf("AEB Controller: empty_mq_counter reached the limit, exiting\n");
    return NULL;
}

/**
 * @brief Prints debug information about the AEB system's internal state.
 * 
 * This function displays the current values of key variables such as relative velocity, 
 * obstacle distance, pedal states, and more.
 */
void print_info()
{
    printf("relative_velocity: %lf\n", aeb_internal_state.relative_velocity);
    printf("has_obstacle: %s\n", aeb_internal_state.has_obstacle ? "true" : "false");
    printf("obstacle_distance: %lf\n", aeb_internal_state.obstacle_distance);
    printf("brake_pedal: %s\n", aeb_internal_state.brake_pedal ? "true" : "false");
    printf("accelerator_pedal: %s\n", aeb_internal_state.accelerator_pedal ? "true" : "false");
    printf("on_off_aeb_system: %s\n", aeb_internal_state.on_off_aeb_system ? "true" : "false");
    printf("Is vehicle in reverse: %s\n", aeb_internal_state.reverseEnabled ? "true" : "false");
}
#endif

/**
 * @brief Translates the received CAN message and calls the appropriate handler 
 * function based on the message identifier.
 * 
 * This function checks the message identifier and calls the appropriate function 
 * to handle the CAN message data.
 * 
 * @param captured_frame The received CAN message to be processed.
 */
void translateAndCallCanMsg(can_msg captured_frame)
{
    switch (captured_frame.identifier)
    {
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
        printf("CAN Identifier unknown\n");
        break;
    }
}

/**
 * @brief Updates the internal state for the brake and accelerator pedals from the received CAN message.
 * 
 * This function updates the brake and accelerator pedals' states based on the received CAN message.
 * 
 * @param captured_frame The captured CAN message containing pedal data.
 */
void updateInternalPedalsState(can_msg captured_frame)
{
    if (captured_frame.dataFrame[0] == 0x00)
    {
        aeb_internal_state.accelerator_pedal = false;
    }
    else if (captured_frame.dataFrame[0] == 0x01)
    {
        aeb_internal_state.accelerator_pedal = true;
    }

    if (captured_frame.dataFrame[1] == 0x00)
    {
        aeb_internal_state.brake_pedal = false;
    }
    else if (captured_frame.dataFrame[1] == 0x01)
    {
        aeb_internal_state.brake_pedal = true;
    }
}

/**
 * @brief Updates the internal speed state based on the received CAN message.
 * 
 * This function updates the internal speed (relative velocity) from the received CAN message.
 * 
 * @param captured_frame The captured CAN message containing speed data.
 */
void updateInternalSpeedState(can_msg captured_frame)
{
    unsigned int data_speed; // used for can frame conversion
    double new_internal_speed = 0.0;
    unsigned int data_acel; // used for can frame conversion for acceleration
    double new_internal_acel = 0.0;

    // update internal data according to the relative velocity detected by the sensor
    if (captured_frame.dataFrame[0] == 0xFE && captured_frame.dataFrame[1] == 0xFF)
    { // DBC: Clear Data
        new_internal_speed = 0.0;
    }
    else if (captured_frame.dataFrame[0] == 0xFF && captured_frame.dataFrame[1] == 0xFF)
    { // DBC: Do nothing
        ;
    }
    else
    {
        // Conversion from CAN data frame, according to dbc in the requirement file
        // [SwR-10]
        data_speed = captured_frame.dataFrame[0] + (captured_frame.dataFrame[1] << 8);
        new_internal_speed = data_speed * RES_SPEED_S;
    }

    if (new_internal_speed > 251.0)
    { // DBC: Max value constraint
        new_internal_speed = 251.0;
    }

    aeb_internal_state.relative_velocity = new_internal_speed;

    // update internal data according to the movement direction reported by the sensor
    if (captured_frame.dataFrame[2] == 0x00)
    {
        aeb_internal_state.reverseEnabled = false;
    }
    else if (captured_frame.dataFrame[2] == 0x01)
    {
        aeb_internal_state.reverseEnabled = true;
    }

    // update internal data according to the relative acceleration detected by the sensor
    if (captured_frame.dataFrame[3] == 0xFE && captured_frame.dataFrame[4] == 0xFF)
    { // DBC: Clear Data
        new_internal_acel = 0.0;
    }
    else if (captured_frame.dataFrame[3] == 0xFF && captured_frame.dataFrame[4] == 0xFF)
    { // DBC: Do nothing
        ;
    }
    else
    {
        // Conversion from CAN data frame, according to dbc in the requirement file 
        // [SwR-10]
        data_acel = captured_frame.dataFrame[3] + (captured_frame.dataFrame[4] << 8);
        new_internal_acel = (data_acel + OFFSET_ACCELERATION_S) * RES_ACCELERATION_S;
    }

    if(captured_frame.dataFrame[5] == 0x01){
        new_internal_acel *= -1;
    } else {
        ;
    }

    if (new_internal_acel > MAX_ACCELERATION_S)
    { // DBC: Max value constraint
        new_internal_acel = MAX_ACCELERATION_S;
    } else if(new_internal_acel < MIN_ACCELERATION_S){
        // DBC: Min value constraint
        new_internal_acel = MIN_ACCELERATION_S;
    }

    aeb_internal_state.relative_acceleration = new_internal_acel;

    //printf("acel is: %.3lf\n", aeb_internal_state.relative_acceleration);
}

/**
 * @brief Updates the internal obstacle state based on the received CAN message.
 * 
 * This function updates whether an obstacle is detected and the distance to it based on the received CAN message.
 * 
 * @param captured_frame The captured CAN message containing obstacle data.
 */
void updateInternalObstacleState(can_msg captured_frame)
{
    unsigned int data_distance; // used for can frame conversion
    double new_internal_distance = 0.0;

    // Check if there is an obstacle
    if (captured_frame.dataFrame[2] == 0x00)
    {
        aeb_internal_state.has_obstacle = false;
        aeb_internal_state.obstacle_distance = 300.0; // Set max distance when no obstacle is detected
        return;
    }
    else if (captured_frame.dataFrame[2] == 0x01)
    {
        aeb_internal_state.has_obstacle = true;
    }

    // Handle special cases for clearing data or doing nothing
    if (captured_frame.dataFrame[1] == 0xFF && captured_frame.dataFrame[0] == 0xFE)
    { // DBC: Clear Data
        new_internal_distance = 300.0; // Set to max distance
    }
    else if (captured_frame.dataFrame[1] == 0xFF && captured_frame.dataFrame[0] == 0xFF)
    { // DBC: Do nothing
        ;
    }
    else
    {
        // Conversion from CAN data frame, according to dbc in the requirement file
        data_distance = captured_frame.dataFrame[0] + (captured_frame.dataFrame[1] << 8);
        new_internal_distance = data_distance * RES_OBSTACLE_S;
    }

    // Apply the max distance constraint
    if (new_internal_distance > 300.0)
    { // DBC: Max value constraint
        new_internal_distance = 300.0;
    }

    // Update internal state with calculated or reset distance
    aeb_internal_state.obstacle_distance = new_internal_distance;
}

/**
 * @brief Updates the internal car state based on the received CAN message.
 * 
 * This function updates the status of the AEB system (on/off) based on the data in the CAN message.
 * 
 * @param captured_frame The captured CAN message containing car state data.
 */
void updateInternalCarCState(can_msg captured_frame)
{
    if (captured_frame.dataFrame[0] == 0x00)
    {
        aeb_internal_state.on_off_aeb_system = false;
    }
    else if (captured_frame.dataFrame[0] == 0x01)
    {
        aeb_internal_state.on_off_aeb_system = true;
    }
}

/**
 * @brief Generates the appropriate CAN message for the AEB system state.
 * 
 * This function creates a CAN message to be sent to the actuators based on the current 
 * state of the AEB system (e.g., brake, alarm, etc.).
 * 
 * @param state The current state of the AEB system.
 * @return The generated CAN message.
 */
can_msg updateCanMsgOutput(aeb_controller_state state)
{
    can_msg aux = {.identifier = ID_AEB_S, .dataFrame = BASE_DATA_FRAME};

    switch (state)
    {
    case AEB_STATE_BRAKE:
        aux.dataFrame[0] = 0x01; // activate warning system
        aux.dataFrame[1] = 0x01; // activate braking system
        break;
    case AEB_STATE_ALARM:
        aux.dataFrame[0] = 0x01; // activate warning system
        aux.dataFrame[1] = 0x00; // don't activate braking system
        break;
    case AEB_STATE_ACTIVE:
        aux.dataFrame[0] = 0x00; // don't activate warning system
        aux.dataFrame[1] = 0x00; // don't activate braking system
        break;
    case AEB_STATE_STANDBY:
        aux.dataFrame[0] = 0x00; // don't activate warning system
        aux.dataFrame[1] = 0x00; // don't activate braking system
        break;
    default:
        break;
    }

    return aux;
}

/**
 * @brief Determines the current state of the AEB system based on sensor input and TTC.
 * 
 * This function evaluates the current AEB state based on multiple sensor 
 * parameters, such as relative velocity, obstacle presence, and TTC (Time to Collision).
 * 
 * @param aeb_internal_state The current sensor data for the AEB system.
 * @param ttc The time-to-collision value, calculated based on obstacle distance and vehicle speed.
 * @return The current AEB state.
 */
aeb_controller_state getAEBState(sensors_input_data aeb_internal_state, double ttc) 
{// Abstraction according to [SwR-12]
    if (aeb_internal_state.on_off_aeb_system == false)
        return AEB_STATE_STANDBY;
    if (aeb_internal_state.relative_velocity < MIN_SPD_ENABLED && aeb_internal_state.reverseEnabled == false) // Required by [SwR-7][SwR-16]
        return AEB_STATE_STANDBY;
    // Required by [SwR-7][SwR-16]
    if (aeb_internal_state.relative_velocity > MAX_SPD_ENABLED && aeb_internal_state.reverseEnabled == false) 
        return AEB_STATE_STANDBY;
    if (aeb_internal_state.brake_pedal == false && aeb_internal_state.accelerator_pedal == false)
    {
        if (ttc < THRESHOLD_BRAKING)
            return AEB_STATE_BRAKE;
        if (ttc < THRESHOLD_ALARM)
            return AEB_STATE_ALARM;
    }
    return AEB_STATE_ACTIVE; // [SwR-8]
}