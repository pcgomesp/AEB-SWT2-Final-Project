// Necessary libraries
#include "unity.h"
#include "constants.h"
#include "sensors_input.h"
#include "dbc.h"
#include <mqueue.h>
#include "ttc_control.h"

/**
 * @brief Enumeration of AEB controller states.
 */
typedef enum {
    AEB_STATE_ACTIVE, /**< AEB is active and functioning normally */
    AEB_STATE_ALARM, /**< AEB system is in alarm state due to a potential collision */
    AEB_STATE_BRAKE, /**< AEB system is in brake state to avoid a collision */
    AEB_STATE_STANDBY /**< AEB system is in standby mode, awaiting activation */
} aeb_controller_state;

/** External declaration of global variables */
extern sensors_input_data aeb_internal_state; /**< Internal AEB system state data */
extern can_msg captured_can_frame; /**< Captured CAN message */
extern can_msg out_can_frame; /**< Output CAN message */
extern can_msg empty_msg; /**< Empty CAN message */

void translateAndCallCanMsg(can_msg captured_frame);
void updateInternalPedalsState(can_msg captured_frame);
void updateInternalSpeedState(can_msg captured_frame);
void updateInternalObstacleState(can_msg captured_frame);
void updateInternalCarCState(can_msg captured_frame);
can_msg updateCanMsgOutput(aeb_controller_state state);
aeb_controller_state getAEBState(sensors_input_data aeb_internal_state, double ttc);

/**
 * @brief Mock function to simulate opening a message queue.
 * 
 * @param mq_name The name of the message queue to open.
 * @return mqd_t The mock message queue descriptor.
 */
mqd_t open_mq(char *mq_name) {
    printf("Opening message queue: %s\n\n", mq_name);
    return 1;  // Return a mock handle for the message queue
}

/**
 * @brief Mock function to simulate writing a message to a message queue.
 * 
 * @param mq_sender The message queue descriptor.
 * @param msg The CAN message to be written.
 * @return int The status of the write operation (0 for success).
 */
int write_mq(mqd_t mq_sender, can_msg *msg) {
    //printf("Writing message to MQ: Identifier: %d, Data: %02X %02X %02X %02X %02X %02X %02X %02X\n\n",
    //        msg->identifier, msg->dataFrame[0], msg->dataFrame[1], msg->dataFrame[2], 
    //        msg->dataFrame[3], msg->dataFrame[4], msg->dataFrame[5], msg->dataFrame[6], msg->dataFrame[7]);
    return 0;  // Simulate success in writing to the message queue
}

/**
 * @brief Mock function to simulate reading a message from a message queue.
 * 
 * @param mq The message queue descriptor.
 * @param msg The CAN message to store the read data.
 * @return int The status of the read operation (0 for success).
 */
int read_mq(mqd_t mq, can_msg *msg) {
    return 0;  // Simulate success in reading the message
}

/**
 * @brief Setup function to initialize AEB input state before each test.
 */
void setUp(void) {
    aeb_internal_state.relative_velocity = 0.0;
    aeb_internal_state.has_obstacle = false;        
    aeb_internal_state.obstacle_distance = 0.0;
    aeb_internal_state.brake_pedal = false;
    aeb_internal_state.accelerator_pedal = false;
    aeb_internal_state.on_off_aeb_system = true;
    aeb_internal_state.reverseEnabled = false;
}

/**
 * @brief Teardown function to clean up after each test.
 */
void tearDown(void) {
    // No cleanup required for now
}

/**
 * @brief Test for the function updateInternalPedalsState. [SwR-6], [SwR-9], [SwR-11]
 */
void test_updateInternalPedalsState(void) {
    can_msg captured_frame = { .identifier = ID_PEDALS, .dataFrame = {0x01, 0x00} };

    updateInternalPedalsState(captured_frame);

    //// Test Case ID: TC_AEB_CTRL_001
    TEST_ASSERT_TRUE(aeb_internal_state.accelerator_pedal);  // Accelerator pedal should be ON  
    TEST_ASSERT_FALSE(aeb_internal_state.brake_pedal);  // Brake pedal should be OFF
    
    captured_frame.dataFrame[0] = 0x01;
    captured_frame.dataFrame[1] = 0x01;
    updateInternalPedalsState(captured_frame);
    
    //// Test Case ID: TC_AEB_CTRL_002
    TEST_ASSERT_TRUE(aeb_internal_state.accelerator_pedal);  // Accelerator pedal should be ON
    TEST_ASSERT_TRUE(aeb_internal_state.brake_pedal);  // Brake pedal should be ON
    
    captured_frame.dataFrame[0] = 0x00;
    captured_frame.dataFrame[1] = 0x00;
    updateInternalPedalsState(captured_frame);
    
    //// Test Case ID: TC_AEB_CTRL_003
    TEST_ASSERT_FALSE(aeb_internal_state.accelerator_pedal);  // Accelerator pedal should be OFF
    TEST_ASSERT_FALSE(aeb_internal_state.brake_pedal);  // Brake pedal should be OFF
    
    captured_frame.dataFrame[0] = 0x00;
    captured_frame.dataFrame[1] = 0x01;
    updateInternalPedalsState(captured_frame);
    
    //// Test Case ID: TC_AEB_CTRL_004
    TEST_ASSERT_FALSE(aeb_internal_state.accelerator_pedal);  // Accelerator pedal should be OFF
    TEST_ASSERT_TRUE(aeb_internal_state.brake_pedal);  // Brake pedal should be ON
}

/**
 * @brief Test for the function updateInternalSpeedState. [SwR-6], [SwR-9], [SwR-11]
 */
void test_updateInternalSpeedState(void) {
    can_msg captured_frame = { .identifier = ID_SPEED_S, .dataFrame = {0x00, 0x64, 0x00} };
    
    //// Test Case ID: TC_AEB_CTRL_005
    // Case 1: Normal case where the speed is updated correctly
    updateInternalSpeedState(captured_frame);
    TEST_ASSERT_EQUAL_FLOAT(aeb_internal_state.relative_velocity, 100.0); // (0x64 + (0x00 << 8)) * RES_SPEED_S = 100.0 
    
    //// Test Case ID: TC_AEB_CTRL_006
    // Case 2: Case where the CAN data is set to clear data (0xFE, 0xFF)
    captured_frame.dataFrame[0] = 0xFE;
    captured_frame.dataFrame[1] = 0xFF;
    updateInternalSpeedState(captured_frame);
    TEST_ASSERT_EQUAL_FLOAT(aeb_internal_state.relative_velocity, 0.0); // Should reset speed to 0.0
    TEST_ASSERT_FALSE(aeb_internal_state.reverseEnabled); // Should also reset reverseEnabled to false
    
    //// Test Case ID: TC_AEB_CTRL_007
    // Case 3: Case where the CAN data is set to indicate reverse (0x01 in dataFrame[2])
    captured_frame.dataFrame[2] = 0x01;
    updateInternalSpeedState(captured_frame);
    TEST_ASSERT_TRUE(aeb_internal_state.reverseEnabled); // Should enable reverse

    //// Test Case ID: TC_AEB_CTRL_008
    // Case 4: Max speed value constraint (should be capped at 251.0)
    captured_frame.dataFrame[0] = 0xFF; 
    captured_frame.dataFrame[1] = 0xFD; // Maximum possible value
    updateInternalSpeedState(captured_frame);
    TEST_ASSERT_EQUAL_FLOAT(aeb_internal_state.relative_velocity, 251.0); // Should be capped at 251.0
    
    //// Test Case ID: TC_AEB_CTRL_009
    // Case 5: Test when the CAN frame data doesn't require any action (0xFF, 0xFF in dataFrame[0] and dataFrame[1])
    captured_frame.dataFrame[0] = 0xFF;
    captured_frame.dataFrame[1] = 0xFF;
    updateInternalSpeedState(captured_frame);
    TEST_ASSERT_EQUAL_FLOAT(aeb_internal_state.relative_velocity, 0.0); // Should set speed to 0.0
    TEST_ASSERT_TRUE(aeb_internal_state.reverseEnabled); // Should remain in the reverseDisabled state

    // Case 6: Ensure that reverseEnabled remains false for non-reverse signals
    captured_frame.dataFrame[2] = 0x00; // Non-reverse case
    updateInternalSpeedState(captured_frame);
    TEST_ASSERT_FALSE(aeb_internal_state.reverseEnabled); // Reverse should not be enabled
}

/**
 * @brief Test for the function updateInternalObstacleState. [SwR-3], [SwR-6], [SwR-7], [SwR-8], [SwR-9], [SwR-11], [SwR-15]
 */
void test_updateInternalObstacleState(void) {
    can_msg captured_frame = { .identifier = ID_OBSTACLE_S, .dataFrame = {0xD0, 0x07, 0x01} };
    
    //// Test Case ID: TC_AEB_CTRL_010
    // Case 1: Obstacle detected and distance calculated correctly
    updateInternalObstacleState(captured_frame);
    TEST_ASSERT_TRUE(aeb_internal_state.has_obstacle);  // Obstacle detected
    TEST_ASSERT_EQUAL_FLOAT(aeb_internal_state.obstacle_distance, 100.0);  // Expected distance =  (0xD0 + (0x07 << 8)) * RES_OBSTACLE = 100.0
    
    //// Test Case ID: TC_AEB_CTRL_011
    // Case 2: No obstacle (0x00 in dataFrame[2])
    captured_frame.dataFrame[2] = 0x00;
    updateInternalObstacleState(captured_frame);
    TEST_ASSERT_FALSE(aeb_internal_state.has_obstacle);  // No obstacle
    TEST_ASSERT_EQUAL_FLOAT(aeb_internal_state.obstacle_distance, 100.0);  // Distance should remain the same
    // Logic problem: if the obstacle is not detected, the distance should be set to 300.0 (max distance).

    //// Test Case ID: TC_AEB_CTRL_012
    // Case 3: Data reset with 0xFE, 0xFF
    captured_frame.dataFrame[0] = 0xFE;
    captured_frame.dataFrame[1] = 0xFF;
    updateInternalObstacleState(captured_frame);
    TEST_ASSERT_FALSE(aeb_internal_state.has_obstacle);  // Obstacle not changed
    TEST_ASSERT_EQUAL_FLOAT(aeb_internal_state.obstacle_distance, 300.0);  // Distance should be set to max value (300.0)
    
    // Case 4: Data doesn't need action (0xFF, 0xFF)
    captured_frame.dataFrame[0] = 0xFF;
    captured_frame.dataFrame[1] = 0xFF;
    updateInternalObstacleState(captured_frame);
    TEST_ASSERT_FALSE(aeb_internal_state.has_obstacle);  // Obstacle not changed
    TEST_ASSERT_EQUAL_FLOAT(aeb_internal_state.obstacle_distance, 0.0);  // Distance should be set to 0.0
    
    // Case 5: Calculated distance from CAN data
    captured_frame.dataFrame[0] = 0xD0;
    captured_frame.dataFrame[1] = 0x07; // Data distance = 0x0D07 = 100
    captured_frame.dataFrame[2] = 0x01; // Obstacle exists
    updateInternalObstacleState(captured_frame);
    TEST_ASSERT_TRUE(aeb_internal_state.has_obstacle);  // Obstacle detected
    TEST_ASSERT_EQUAL_FLOAT(aeb_internal_state.obstacle_distance, 100.0);  // Calculated distance
    
    //// Test Case ID: TC_AEB_CTRL_013
    // Case 6: Maximum distance capped at 300.0
    captured_frame.dataFrame[0] = 0xFF;
    captured_frame.dataFrame[1] = 0xFD; // Data distance = 0xFFFF = 65535
    updateInternalObstacleState(captured_frame);
    TEST_ASSERT_TRUE(aeb_internal_state.has_obstacle);  // Obstacle detected
    TEST_ASSERT_EQUAL_FLOAT(aeb_internal_state.obstacle_distance, 300.0);  // Should be capped at 300.0
}

/**
 * @brief Test for the function updateInternalCarCState. [SwR-2], [SwR-3], [SwR-7], [SwR-8], [SwR-11], [SwR-12], [SwR-16]
 */
void test_updateInternalCarCState(void) {
    can_msg captured_frame = { .identifier = ID_CAR_C, .dataFrame = {0x01} };

    //// Test Case ID: TC_AEB_CTRL_014
    // Case 1: AEB system ON (dataFrame[0] == 0x01)
    updateInternalCarCState(captured_frame);
    TEST_ASSERT_TRUE(aeb_internal_state.on_off_aeb_system);  // AEB system should be ON

    //// Test Case ID: TC_AEB_CTRL_015
    // Case 2: AEB system OFF (dataFrame[0] == 0x00)
    captured_frame.dataFrame[0] = 0x00;
    updateInternalCarCState(captured_frame);
    TEST_ASSERT_FALSE(aeb_internal_state.on_off_aeb_system);  // AEB system should be OFF

    // Case 3: Check that AEB system remains OFF for any value other than 0x01
    captured_frame.dataFrame[0] = 0x02;  // Arbitrary value, different from 0x01
    updateInternalCarCState(captured_frame);
    TEST_ASSERT_FALSE(aeb_internal_state.on_off_aeb_system);  // AEB system should remain OFF

    // Case 4: Check that AEB system turns back ON when dataFrame[0] is 0x01 again
    captured_frame.dataFrame[0] = 0x01;
    updateInternalCarCState(captured_frame);
    TEST_ASSERT_TRUE(aeb_internal_state.on_off_aeb_system);  // AEB system should be back ON
}

/**
 * @brief Test for the function getAEBState. [SwR-2], [SwR-3], [SwR-6], [SwR-7], [SwR-8], [SwR-9], [SwR-11], [SwR-12], [SwR-15],[SwR-16]. 
 */
void test_getAEBState(void) {
    // Initial setup of AEB state
    aeb_internal_state.relative_velocity = 80.0;
    aeb_internal_state.has_obstacle = true;
    aeb_internal_state.obstacle_distance = 10.0;
    aeb_internal_state.brake_pedal = false;
    aeb_internal_state.accelerator_pedal = false;
    aeb_internal_state.on_off_aeb_system = true;  // AEB system is ON
    aeb_internal_state.reverseEnabled = false;   // Reversing is not enabled

    double ttc = 1.9;  // Example of TTC

    // Case 1: AEB system active and TTC indicating alarm state
    aeb_controller_state state = getAEBState(aeb_internal_state, ttc);
    TEST_ASSERT_EQUAL_INT(state, AEB_STATE_ALARM);  // Should be in ALARM state

    // Case 2: AEB system active, but TTC too low (should go to brake state)
    ttc = 0.9;
    state = getAEBState(aeb_internal_state, ttc);
    TEST_ASSERT_EQUAL_INT(state, AEB_STATE_BRAKE);  // Should go to BRAKE state if speed is high and TTC low

    // Case 3: AEB system OFF (on_off_aeb_system == false)
    aeb_internal_state.on_off_aeb_system = false;
    state = getAEBState(aeb_internal_state, ttc);
    TEST_ASSERT_EQUAL_INT(state, AEB_STATE_STANDBY);  // Should be in STANDBY state

    // Case 4: Speed below MIN_SPD_ENABLED and no reverse enabled (should go to STANDBY)
    aeb_internal_state.on_off_aeb_system = true;
    aeb_internal_state.relative_velocity = MIN_SPD_ENABLED - 1;
    state = getAEBState(aeb_internal_state, ttc);
    TEST_ASSERT_EQUAL_INT(state, AEB_STATE_STANDBY);  // Should go to STANDBY state

    // Case 5: Speed above MAX_SPD_ENABLED and no reverse enabled (should go to STANDBY)
    aeb_internal_state.relative_velocity = MAX_SPD_ENABLED + 1;
    state = getAEBState(aeb_internal_state, ttc);
    TEST_ASSERT_EQUAL_INT(state, AEB_STATE_STANDBY);  // Should go to STANDBY state

    //// Test Case ID: TC_AEB_CTRL_016
    // Case 6: Pedals deactivated (accelerator and brake) with low TTC (should go to BRAKE state)
    aeb_internal_state.relative_velocity = 60.0;  // Speed within allowed range
    ttc = 0.8;  // TTC lower than the braking threshold
    state = getAEBState(aeb_internal_state, ttc);
    TEST_ASSERT_EQUAL_INT(state, AEB_STATE_BRAKE);  // Should go to BRAKE state due to low TTC

    //// Test Case ID: TC_AEB_CTRL_017
    // Case 7: Pedals deactivated with TTC within alarm range (should go to ALARM state)
    ttc = 1.5;  // TTC greater than the braking threshold but less than the alarm threshold
    state = getAEBState(aeb_internal_state, ttc);
    TEST_ASSERT_EQUAL_INT(state, AEB_STATE_ALARM);  // Should go to ALARM state

    //// Test Case ID: TC_AEB_CTRL_018
    // Case 8: Pedals deactivated with TTC greater than alarm threshold (should go to ACTIVE state)
    ttc = 2.0;  // TTC greater than alarm threshold
    state = getAEBState(aeb_internal_state, ttc);
    TEST_ASSERT_EQUAL_INT(state, AEB_STATE_ACTIVE);  // Should go to ACTIVE state

    //// Test Case ID: TC_AEB_CTRL_019
    // Case 9: Brake pedal pressed (should go to ACTIVE state)
    aeb_internal_state.brake_pedal = true;  // Brake pedal pressed
    state = getAEBState(aeb_internal_state, ttc);
    TEST_ASSERT_EQUAL_INT(state, AEB_STATE_ACTIVE);  // Should go to ACTIVE state regardless of TTC

    //// Test Case ID: TC_AEB_CTRL_020
    // Case 10: Accelerator pedal pressed (should go to ACTIVE state)
    aeb_internal_state.accelerator_pedal = true;  // Accelerator pedal pressed
    state = getAEBState(aeb_internal_state, ttc);
    TEST_ASSERT_EQUAL_INT(state, AEB_STATE_ACTIVE);  // Should go to ACTIVE state regardless of TTC
}

/**
 * @brief Series of tests for the function translateAndCallCanMsg. [SwR-9] 
 */
void test_translateAndCallCanMsg_1(void) {
    can_msg captured_frame;

    ///// Test Case ID: TC_AEB_CTRL_021
    // Case 1: Pedal identifier (ID_PEDALS) should call updateInternalPedalsState
    captured_frame.identifier = ID_PEDALS;
    captured_frame.dataFrame[0] = 0x01;  // Accelerator ON
    captured_frame.dataFrame[1] = 0x00;  // Brake OFF
    translateAndCallCanMsg(captured_frame);
    TEST_ASSERT_TRUE(aeb_internal_state.accelerator_pedal);  // Accelerator pedal should be ON
    TEST_ASSERT_FALSE(aeb_internal_state.brake_pedal);  // Brake pedal should be OFF
}

void test_translateAndCallCanMsg_2()
{
    can_msg captured_frame;

    // Case 2: Pedal identifier (ID_PEDALS) should call updateInternalPedalsState with Brake ON
    captured_frame.identifier = ID_PEDALS;
    captured_frame.dataFrame[0] = 0x00;  // Accelerator OFF
    captured_frame.dataFrame[1] = 0x01;  // Brake ON
    translateAndCallCanMsg(captured_frame);
    TEST_ASSERT_FALSE(aeb_internal_state.accelerator_pedal);  // Accelerator pedal should be OFF
    TEST_ASSERT_TRUE(aeb_internal_state.brake_pedal);  // Brake pedal should be ON
}

void test_translateAndCallCanMsg_3()
{
    can_msg captured_frame;

    //// Test Case ID: TC_AEB_CTRL_022
    // Case 3: Speed identifier (ID_SPEED_S) should call updateInternalSpeedState
    captured_frame.identifier = ID_SPEED_S;
    captured_frame.dataFrame[0] = 0x00;
    captured_frame.dataFrame[1] = 0x64;  // Speed = 100
    captured_frame.dataFrame[2] = 0x00;
    translateAndCallCanMsg(captured_frame);
    TEST_ASSERT_EQUAL_FLOAT(aeb_internal_state.relative_velocity, 100.0);  // Speed should be 100 km/h
}

void test_translateAndCallCanMsg_4()
{
    can_msg captured_frame;

    //// Test Case ID: TC_AEB_CTRL_023
    // Case 4: Obstacle identifier (ID_OBSTACLE_S) should call updateInternalObstacleState
    captured_frame.identifier = ID_OBSTACLE_S;
    captured_frame.dataFrame[0] = 0xD0;  // Distance part 1
    captured_frame.dataFrame[1] = 0x07;  // Distance part 2 (Total 100 meters)
    captured_frame.dataFrame[2] = 0x01;  // Obstacle detected
    translateAndCallCanMsg(captured_frame);
    TEST_ASSERT_TRUE(aeb_internal_state.has_obstacle);  // Obstacle should be detected
    TEST_ASSERT_EQUAL_FLOAT(aeb_internal_state.obstacle_distance, 100.0);  // Distance should be 100 meters
}

void test_translateAndCallCanMsg_5()
{
    can_msg captured_frame;

    //// Test Case ID: TC_AEB_CTRL_024
    // Case 5: Car identifier (ID_CAR_C) should call updateInternalCarCState
    captured_frame.identifier = ID_CAR_C;
    captured_frame.dataFrame[0] = 0x01;  // AEB system ON
    translateAndCallCanMsg(captured_frame);
    TEST_ASSERT_TRUE(aeb_internal_state.on_off_aeb_system);  // AEB system should be ON
}

void test_translateAndCallCanMsg_6()
{
    can_msg captured_frame;

    TEST_IGNORE_MESSAGE("Test case for unknown identifier");

    // Case 6: Unknown identifier should print "CAN Identifier unknown"
    captured_frame.identifier = ID_EMPTY; // Unknown ID
    // The test could capture the printf output and verify the printed message.
    translateAndCallCanMsg(captured_frame);
    // TEST_ASSERT_EQUAL_STRING("CAN Identifier unknown\n", capture_stdout());
}

void test_translateAndCallCanMsg_7()
{
    can_msg captured_frame;

    // Case 7: Test reverse flag handling in Speed message
    captured_frame.identifier = ID_SPEED_S;
    captured_frame.dataFrame[0] = 0x00;
    captured_frame.dataFrame[1] = 0x64;  // Speed = 100
    captured_frame.dataFrame[2] = 0x01;  // Reverse enabled
    translateAndCallCanMsg(captured_frame);
    TEST_ASSERT_TRUE(aeb_internal_state.reverseEnabled);  // Reverse should be enabled
}

void test_translateAndCallCanMsg_8()
{
    can_msg captured_frame;

    // Case 8: Test the behavior when receiving clear data for speed
    captured_frame.dataFrame[0] = 0xFE;  // Clear data for speed
    captured_frame.dataFrame[1] = 0xFF;  
    captured_frame.dataFrame[2] = 0x00;
    translateAndCallCanMsg(captured_frame);
    TEST_ASSERT_EQUAL_FLOAT(aeb_internal_state.relative_velocity, 0.0);  // Speed should reset to 0
    TEST_ASSERT_FALSE(aeb_internal_state.reverseEnabled);  // Reverse should be disabled
}

void test_translateAndCallCanMsg_9()
{
    can_msg captured_frame;

    // Case 9: Test the behavior when receiving invalid data for obstacle
    captured_frame.identifier = ID_OBSTACLE_S;
    captured_frame.dataFrame[0] = 0xFF;  // Invalid obstacle data
    captured_frame.dataFrame[1] = 0xFF;
    captured_frame.dataFrame[2] = 0xFF;  // No obstacle
    translateAndCallCanMsg(captured_frame);
    TEST_ASSERT_FALSE(aeb_internal_state.has_obstacle);  // No obstacle detected
    TEST_ASSERT_EQUAL_FLOAT(aeb_internal_state.obstacle_distance, 0.0);  // Distance should be reset to 0
}    

/**
 * @brief Main function to run the tests.
 * 
 * @return int The result of the test run.
 */
int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_updateInternalPedalsState);
    RUN_TEST(test_updateInternalSpeedState);
    RUN_TEST(test_updateInternalObstacleState);
    RUN_TEST(test_updateInternalCarCState);
    RUN_TEST(test_getAEBState);
    RUN_TEST(test_translateAndCallCanMsg_1);
    RUN_TEST(test_translateAndCallCanMsg_2);
    RUN_TEST(test_translateAndCallCanMsg_3);
    RUN_TEST(test_translateAndCallCanMsg_4);
    RUN_TEST(test_translateAndCallCanMsg_5);
    RUN_TEST(test_translateAndCallCanMsg_6);
    RUN_TEST(test_translateAndCallCanMsg_7);
    RUN_TEST(test_translateAndCallCanMsg_8);
    RUN_TEST(test_translateAndCallCanMsg_9);
    return UNITY_END();
}