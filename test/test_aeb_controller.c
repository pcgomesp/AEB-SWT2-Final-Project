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
 * @brief Helper function to check the pedal state.
 * 
 * @param expected_accelerator Expected state of the accelerator pedal.
 * @param expected_brake Expected state of the brake pedal.
 */
void checkPedalState(bool expected_accelerator, bool expected_brake) {
    if (expected_accelerator){
        TEST_ASSERT_TRUE(aeb_internal_state.accelerator_pedal);
    } else {
        TEST_ASSERT_FALSE(aeb_internal_state.accelerator_pedal);
    }
    
    if (expected_brake){
        TEST_ASSERT_TRUE(aeb_internal_state.brake_pedal);
    } else {
        TEST_ASSERT_FALSE(aeb_internal_state.brake_pedal);
    }
}

/**
 * @brief Test Case TC_AEB_CTRL_001: Accelerator pedal ON, Brake pedal OFF.
 */
void test_TC_AEB_CTRL_001(void) {
    can_msg captured_frame = { .identifier = ID_PEDALS, .dataFrame = {0x01, 0x00} };

    updateInternalPedalsState(captured_frame);

    // Test: Accelerator pedal should be ON, Brake pedal should be OFF
    checkPedalState(true, false);
}

/**
 * @brief Test Case TC_AEB_CTRL_002: Accelerator pedal ON, Brake pedal ON.
 */
void test_TC_AEB_CTRL_002(void) {
    can_msg captured_frame = { .identifier = ID_PEDALS, .dataFrame = {0x01, 0x01} };

    updateInternalPedalsState(captured_frame);

    // Test: Accelerator pedal should be ON, Brake pedal should be ON
    checkPedalState(true, true);
}

/**
 * @brief Test Case TC_AEB_CTRL_003: Accelerator pedal OFF, Brake pedal OFF.
 */
void test_TC_AEB_CTRL_003(void) {
    can_msg captured_frame = { .identifier = ID_PEDALS, .dataFrame = {0x00, 0x00} };

    updateInternalPedalsState(captured_frame);

    // Test: Accelerator pedal should be OFF, Brake pedal should be OFF
    checkPedalState(false, false);
}

/**
 * @brief Test Case TC_AEB_CTRL_004: Accelerator pedal OFF, Brake pedal ON.
 */
void test_TC_AEB_CTRL_004(void) {
    can_msg captured_frame = { .identifier = ID_PEDALS, .dataFrame = {0x00, 0x01} };

    updateInternalPedalsState(captured_frame);

    // Test: Accelerator pedal should be OFF, Brake pedal should be ON
    checkPedalState(false, true);
}

/**
 * @brief Helper function to check the relative velocity and reverseEnabled state.
 * 
 * @param expected_velocity Expected relative velocity.
 * @param expected_reverse_enabled Expected state of reverseEnabled.
 */
void checkSpeedState(float expected_velocity, bool expected_reverse_enabled) {
    if (expected_reverse_enabled){
        TEST_ASSERT_TRUE(aeb_internal_state.reverseEnabled);
    } else{
        TEST_ASSERT_FALSE(aeb_internal_state.reverseEnabled);
    }
    TEST_ASSERT_EQUAL_FLOAT(expected_velocity, aeb_internal_state.relative_velocity);
}

/**
 * @brief Test Case TC_AEB_CTRL_005: Normal case where the speed is updated correctly.
 */
void test_TC_AEB_CTRL_005(void) {
    can_msg captured_frame = { .identifier = ID_SPEED_S, .dataFrame = {0x00, 0x64, 0x00} };

    updateInternalSpeedState(captured_frame);

    // Test: Speed should be updated correctly (0x64 + (0x00 << 8)) * RES_SPEED_S = 100.0
    checkSpeedState(100.0, false);
}

/**
 * @brief Test Case TC_AEB_CTRL_006: Case where the CAN data is set to clear data (0xFE, 0xFF).
 */
void test_TC_AEB_CTRL_006(void) {
    can_msg captured_frame = { .identifier = ID_SPEED_S, .dataFrame = {0xFE, 0xFF, 0x00} };

    updateInternalSpeedState(captured_frame);

    // Test: Speed should reset to 0.0, and reverseEnabled should be false
    checkSpeedState(0.0, false);
}

/**
 * @brief Test Case TC_AEB_CTRL_007: Case where the CAN data is set to indicate 
 * reverse (0x01 in dataFrame[2]).
 */
void test_TC_AEB_CTRL_007(void) {
    can_msg captured_frame = { .identifier = ID_SPEED_S, .dataFrame = {0x00, 0x14, 0x01} };

    updateInternalSpeedState(captured_frame);

    // Test: Reverse should be enabled (0x00 + (0x14 << 8)) * RES_SPEED_S = 20.0
    checkSpeedState(20.0, true);
}

/**
 * @brief Test Case TC_AEB_CTRL_008: Max speed value constraint (should be capped at 251.0).
 */
void test_TC_AEB_CTRL_008(void) {
    can_msg captured_frame = { .identifier = ID_SPEED_S, .dataFrame = {0xFF, 0xFD, 0x00} };

    updateInternalSpeedState(captured_frame);

    // Test: Speed should be capped at 251.0
    checkSpeedState(251.0, false);
}

/**
 * @brief Test Case TC_AEB_CTRL_009: Test when the CAN frame data doesn't require any 
 * action (0xFF, 0xFF in dataFrame[0] and dataFrame[1]).
 */
void test_TC_AEB_CTRL_009(void) {
    can_msg captured_frame = { .identifier = ID_SPEED_S, .dataFrame = {0xFF, 0xFF, 0x00} };

    updateInternalSpeedState(captured_frame);

    // Test: Speed should be 0.0, reverseEnabled should remain false
    checkSpeedState(0.0, false);
}

/**
 * @brief Test Case: Ensure that reverseEnabled remains false for non-reverse signals.
 */
void test_TC_AEB_CTRL_X01(void) {
    can_msg captured_frame = { .identifier = ID_SPEED_S, .dataFrame = {0x00, 0x64, 0x03} };

    updateInternalSpeedState(captured_frame);

    // Test: Reverse should not be enabled for non-reverse signals
    checkSpeedState(100.0, false);
}

/**
 * @brief Helper function to check obstacle state.
 * 
 * @param expected_has_obstacle Expected state of has_obstacle.
 * @param expected_distance Expected obstacle distance.
 */
void checkObstacleState(bool expected_has_obstacle, float expected_distance) {
    if (expected_has_obstacle){
        TEST_ASSERT_TRUE(aeb_internal_state.has_obstacle);
    } else {
        TEST_ASSERT_FALSE(aeb_internal_state.has_obstacle);
    }
    TEST_ASSERT_EQUAL_FLOAT(expected_distance, aeb_internal_state.obstacle_distance);
}

/**
 * @brief Test Case TC_AEB_CTRL_010: Obstacle detected and distance calculated correctly.
 */
void test_TC_AEB_CTRL_010(void) {
    can_msg captured_frame = { .identifier = ID_OBSTACLE_S, .dataFrame = {0xD0, 0x07, 0x01} };

    updateInternalObstacleState(captured_frame);

    // Test: Expected distance = (0xD0 + (0x07 << 8)) * RES_OBSTACLE = 100.0
    checkObstacleState(true, 100.0);
}

/**
 * @brief Test Case TC_AEB_CTRL_011: No obstacle (0x00 in dataFrame[2]).
 */
void test_TC_AEB_CTRL_011(void) {
    can_msg captured_frame = { .identifier = ID_OBSTACLE_S, .dataFrame = {0xD0, 0x07, 0x00} };

    updateInternalObstacleState(captured_frame);

    // Test: No obstacle detected, distance should remain the same (max distance)
    checkObstacleState(false, 300.0); // Max distance when no obstacle is detected
}

/**
 * @brief Test Case TC_AEB_CTRL_012: Data reset with 0xFE, 0xFF.
 */
void test_TC_AEB_CTRL_012(void) {
    can_msg captured_frame = { .identifier = ID_OBSTACLE_S, .dataFrame = {0xFE, 0xFF, 0x00} };

    updateInternalObstacleState(captured_frame);

    // Test: Obstacle not detected, reset distance to max value (300.0)
    checkObstacleState(false, 300.0);
}

/**
 * @brief Test Case: Case where data doesn't need action (0xFF, 0xFF).
 */
void test_TC_AEB_CTRL_X02(void) {
    can_msg captured_frame = { .identifier = ID_OBSTACLE_S, .dataFrame = {0xFF, 0xFF, 0x00} };

    updateInternalObstacleState(captured_frame);

    // Test: Obstacle not detected, distance should be reset to 300.0
    checkObstacleState(false, 300.0);
}

/**
 * @brief Test Case: Calculated distance from CAN data.
 */
void test_TC_AEB_CTRL_X03(void) {
    can_msg captured_frame = { .identifier = ID_OBSTACLE_S, .dataFrame = {0xD0, 0x07, 0x01} };

    updateInternalObstacleState(captured_frame);

    // Test: Obstacle detected, distance should be correctly calculated
    checkObstacleState(true, 100.0);
}

/**
 * @brief Test Case TC_AEB_CTRL_013: Maximum distance capped at 300.0.
 */
void test_TC_AEB_CTRL_013(void) {
    can_msg captured_frame = { .identifier = ID_OBSTACLE_S, .dataFrame = {0xFF, 0xFD, 0x00} };

    updateInternalObstacleState(captured_frame);

    // Test: Distance should be capped at 300.0 when CAN data exceeds max
    checkObstacleState(false, 300.0);
}

/**
 * @brief Helper function to check the AEB system state.
 * 
 * @param expected_state Expected state of the AEB system (ON/OFF).
 */
void checkAEBSystemState(bool expected_state) {
    if (expected_state){
        TEST_ASSERT_TRUE(aeb_internal_state.on_off_aeb_system);
    } else {
        TEST_ASSERT_FALSE(aeb_internal_state.on_off_aeb_system);
    }
}

/**
 * @brief Test Case TC_AEB_CTRL_014: AEB system ON (dataFrame[0] == 0x01).
 */
void test_TC_AEB_CTRL_014(void) {
    can_msg captured_frame = { .identifier = ID_CAR_C, .dataFrame = {0x01} };

    updateInternalCarCState(captured_frame);

    // Test: AEB system should be ON when dataFrame[0] is 0x01
    checkAEBSystemState(true);
}

/**
 * @brief Test Case TC_AEB_CTRL_015: AEB system OFF (dataFrame[0] == 0x00).
 */
void test_TC_AEB_CTRL_015(void) {
    can_msg captured_frame = { .identifier = ID_CAR_C, .dataFrame = {0x00} };

    updateInternalCarCState(captured_frame);

    // Test: AEB system should be OFF when dataFrame[0] is 0x00
    checkAEBSystemState(false);
}

/**
 * @brief Test Case: AEB system remains OFF for any value other than 0x01.
 */
void test_TC_AEB_CTRL_X04(void) {
    can_msg captured_frame = { .identifier = ID_CAR_C, .dataFrame = {0x02} };  // Arbitrary value different from 0x01

    updateInternalCarCState(captured_frame);

    // Test: AEB system should remain OFF for any value other than 0x01
    checkAEBSystemState(false);
}

/**
 * @brief Test Case: AEB system turns back ON when dataFrame[0] is 0x01 again.
 */
void test_TC_AEB_CTRL_X05(void) {
    can_msg captured_frame = { .identifier = ID_CAR_C, .dataFrame = {0x01} };

    updateInternalCarCState(captured_frame);

    // Test: AEB system should be back ON when dataFrame[0] is 0x01 again
    checkAEBSystemState(true);
}

/**
 * @brief Helper function to check the AEB state.
 * 
 * @param expected_state The expected state of the AEB system.
 * @param state The actual state returned by getAEBState.
 */
void checkAEBState(aeb_controller_state expected_state, aeb_controller_state state) {
    TEST_ASSERT_EQUAL_INT(expected_state, state);
}

/**
 * @brief Test Case: AEB system active and TTC indicating alarm state.
 */
void test_TC_AEB_CTRL_X06(void) {
    aeb_internal_state.relative_velocity = MAX_SPD_ENABLED;
    aeb_internal_state.has_obstacle = true;
    aeb_internal_state.obstacle_distance = 10.0;
    aeb_internal_state.on_off_aeb_system = true;
    aeb_internal_state.reverseEnabled = false;

    double ttc = 1.9;
    aeb_controller_state state = getAEBState(aeb_internal_state, ttc);

    // Test: AEB system should be in ALARM state
    checkAEBState(AEB_STATE_ALARM, state);
}

/**
 * @brief Test Case: AEB system active, but TTC too low (should go to brake state).
 */
void test_TC_AEB_CTRL_X07(void) {
    aeb_internal_state.relative_velocity = MAX_SPD_ENABLED;
    aeb_internal_state.has_obstacle = true;
    aeb_internal_state.obstacle_distance = 10.0;
    aeb_internal_state.on_off_aeb_system = true;
    aeb_internal_state.reverseEnabled = false;

    double ttc = 0.9;
    aeb_controller_state state = getAEBState(aeb_internal_state, ttc);

    // Test: AEB system should go to BRAKE state if speed is high and TTC low
    checkAEBState(AEB_STATE_BRAKE, state);
}

/**
 * @brief Test Case: AEB system OFF (on_off_aeb_system == false, high speed).
 */
void test_TC_AEB_CTRL_X08(void) {
    aeb_internal_state.relative_velocity = 80.0;
    aeb_internal_state.has_obstacle = true;
    aeb_internal_state.obstacle_distance = 10.0;
    aeb_internal_state.on_off_aeb_system = false;
    aeb_internal_state.reverseEnabled = false;

    double ttc = 1.9;
    aeb_controller_state state = getAEBState(aeb_internal_state, ttc);

    // Test: AEB system should be in STANDBY state
    checkAEBState(AEB_STATE_STANDBY, state);
}

/**
 * @brief Test Case: Speed below MIN_SPD_ENABLED and no reverse enabled (should go to STANDBY).
 */
void test_TC_AEB_CTRL_X09(void) {
    aeb_internal_state.relative_velocity = MIN_SPD_ENABLED - 1;
    aeb_internal_state.has_obstacle = true;
    aeb_internal_state.obstacle_distance = 10.0;
    aeb_internal_state.on_off_aeb_system = true;
    aeb_internal_state.reverseEnabled = false;

    double ttc = 1.9;
    aeb_controller_state state = getAEBState(aeb_internal_state, ttc);

    // Test: AEB system should go to STANDBY state
    checkAEBState(AEB_STATE_STANDBY, state);
}

/**
 * @brief Test Case: Speed above MAX_SPD_ENABLED and no reverse enabled (should go to STANDBY).
 */
void test_TC_AEB_CTRL_X10(void) {
    aeb_internal_state.relative_velocity = MAX_SPD_ENABLED + 1;
    aeb_internal_state.has_obstacle = true;
    aeb_internal_state.obstacle_distance = 10.0;
    aeb_internal_state.on_off_aeb_system = true;
    aeb_internal_state.reverseEnabled = false;

    double ttc = 1.9;
    aeb_controller_state state = getAEBState(aeb_internal_state, ttc);

    // Test: AEB system should go to STANDBY state
    checkAEBState(AEB_STATE_STANDBY, state);
}

/**
 * @brief Test Case TC_AEB_CTRL_016: Pedals deactivated with low TTC (should go to BRAKE state).
 */
void test_TC_AEB_CTRL_016(void) {
    aeb_internal_state.relative_velocity = 60.0;
    aeb_internal_state.has_obstacle = true;
    aeb_internal_state.obstacle_distance = 10.0;
    aeb_internal_state.on_off_aeb_system = true;
    aeb_internal_state.reverseEnabled = false;
    aeb_internal_state.accelerator_pedal = false;
    aeb_internal_state.brake_pedal = false;

    double ttc = 0.8;  // TTC lower than the braking threshold
    aeb_controller_state state = getAEBState(aeb_internal_state, ttc);

    // Test: AEB system should go to BRAKE state due to low TTC
    checkAEBState(AEB_STATE_BRAKE, state);
}

/**
 * @brief Test Case TC_AEB_CTRL_017: Pedals deactivated with TTC within alarm range (should go to ALARM state).
 */
void test_TC_AEB_CTRL_017(void) {
    aeb_internal_state.relative_velocity = 60.0;
    aeb_internal_state.has_obstacle = true;
    aeb_internal_state.obstacle_distance = 10.0;
    aeb_internal_state.on_off_aeb_system = true;
    aeb_internal_state.reverseEnabled = false;
    aeb_internal_state.accelerator_pedal = false;
    aeb_internal_state.brake_pedal = false;

    double ttc = 1.5;  // TTC greater than the braking threshold but less than the alarm threshold
    aeb_controller_state state = getAEBState(aeb_internal_state, ttc);

    // Test: AEB system should go to ALARM state
    checkAEBState(AEB_STATE_ALARM, state);
}

/**
 * @brief Test Case TC_AEB_CTRL_018: Pedals deactivated with TTC greater than alarm threshold (should go to ACTIVE state).
 */
void test_TC_AEB_CTRL_018(void) {
    aeb_internal_state.relative_velocity = 60.0;
    aeb_internal_state.has_obstacle = true;
    aeb_internal_state.obstacle_distance = 10.0;
    aeb_internal_state.on_off_aeb_system = true;
    aeb_internal_state.reverseEnabled = false;
    aeb_internal_state.accelerator_pedal = false;
    aeb_internal_state.brake_pedal = false;

    double ttc = THRESHOLD_ALARM + 0.1;  // TTC greater than alarm threshold
    aeb_controller_state state = getAEBState(aeb_internal_state, ttc);

    // Test: AEB system should go to ACTIVE state
    checkAEBState(AEB_STATE_ACTIVE, state);
}

/**
 * @brief Test Case TC_AEB_CTRL_019: Brake pedal pressed (should go to ACTIVE state).
 */
void test_TC_AEB_CTRL_019(void) {
    aeb_internal_state.relative_velocity = 60.0;
    aeb_internal_state.has_obstacle = true;
    aeb_internal_state.obstacle_distance = 10.0;
    aeb_internal_state.on_off_aeb_system = true;
    aeb_internal_state.reverseEnabled = false;
    aeb_internal_state.accelerator_pedal = false;
    aeb_internal_state.brake_pedal = true;  // Brake pedal pressed

    double ttc = 1.5;
    aeb_controller_state state = getAEBState(aeb_internal_state, ttc);

    // Test: AEB system should go to ACTIVE state regardless of TTC
    checkAEBState(AEB_STATE_ACTIVE, state);
}

/**
 * @brief Test Case TC_AEB_CTRL_020: Accelerator pedal pressed (should go to ACTIVE state).
 */
void test_TC_AEB_CTRL_020(void) {
    aeb_internal_state.relative_velocity = 60.0;
    aeb_internal_state.has_obstacle = true;
    aeb_internal_state.obstacle_distance = 10.0;
    aeb_internal_state.on_off_aeb_system = true;
    aeb_internal_state.reverseEnabled = false;
    aeb_internal_state.accelerator_pedal = true;  // Accelerator pedal pressed
    aeb_internal_state.brake_pedal = false;

    double ttc = 1.5;
    aeb_controller_state state = getAEBState(aeb_internal_state, ttc);

    // Test: AEB system should go to ACTIVE state regardless of TTC
    checkAEBState(AEB_STATE_ACTIVE, state);
}

/**
 * @brief Test Case TC_AEB_CTRL_021: Pedal identifier (ID_PEDALS) should call updateInternalPedalsState.  
 */
void test_TC_AEB_CTRL_021(void) {
    can_msg captured_frame;

    captured_frame.identifier = ID_PEDALS;
    captured_frame.dataFrame[0] = 0x01;  // Accelerator ON
    captured_frame.dataFrame[1] = 0x00;  // Brake OFF
    translateAndCallCanMsg(captured_frame);
    TEST_ASSERT_TRUE(aeb_internal_state.accelerator_pedal);  // Accelerator pedal should be ON
    TEST_ASSERT_FALSE(aeb_internal_state.brake_pedal);  // Brake pedal should be OFF
}

/**
 * @brief Test Case: Pedal identifier (ID_PEDALS) should call updateInternalPedalsState with Brake ON
 */
void test_TC_AEB_CTRL_X11(void) {
    can_msg captured_frame;

    captured_frame.identifier = ID_PEDALS;
    captured_frame.dataFrame[0] = 0x00;  // Accelerator OFF
    captured_frame.dataFrame[1] = 0x01;  // Brake ON
    translateAndCallCanMsg(captured_frame);
    TEST_ASSERT_FALSE(aeb_internal_state.accelerator_pedal);  // Accelerator pedal should be OFF
    TEST_ASSERT_TRUE(aeb_internal_state.brake_pedal);  // Brake pedal should be ON
}

/**
 * @brief Test Case TC_AEB_CTRL_022: Speed identifier (ID_SPEED_S) should call updateInternalSpeedState
 */
void test_TC_AEB_CTRL_022(void) {
    can_msg captured_frame;

    captured_frame.identifier = ID_SPEED_S;
    captured_frame.dataFrame[0] = 0x00;
    captured_frame.dataFrame[1] = 0x64;
    captured_frame.dataFrame[2] = 0x00;
    translateAndCallCanMsg(captured_frame);
    TEST_ASSERT_EQUAL_FLOAT(aeb_internal_state.relative_velocity, 100.0);  // Speed should be 100 km/h
}
/**
 * @brief Test Case TC_AEB_CTRL_023: Obstacle identifier (ID_OBSTACLE_S) should call updateInternalObstacleState
 */
void test_TC_AEB_CTRL_023(void)
{
    can_msg captured_frame;

    captured_frame.identifier = ID_OBSTACLE_S;
    captured_frame.dataFrame[0] = 0xD0;  // Distance part 1
    captured_frame.dataFrame[1] = 0x07;  // Distance part 2 (Total 100 meters)
    captured_frame.dataFrame[2] = 0x01;  // Obstacle detected
    translateAndCallCanMsg(captured_frame);
    TEST_ASSERT_TRUE(aeb_internal_state.has_obstacle);  // Obstacle should be detected
    TEST_ASSERT_EQUAL_FLOAT(aeb_internal_state.obstacle_distance, 100.0);  // Distance should be 100 meters
}

/**
 * @brief Test Case TC_AEB_CTRL_024: Car identifier (ID_CAR_C) should call updateInternalCarCState
 */
void test_TC_AEB_CTRL_024(void)
{
    can_msg captured_frame;

    captured_frame.identifier = ID_CAR_C;
    captured_frame.dataFrame[0] = 0x01;  // AEB system ON
    translateAndCallCanMsg(captured_frame);
    TEST_ASSERT_TRUE(aeb_internal_state.on_off_aeb_system);  // AEB system should be ON
}

/**
 * @brief Test Case: Unknown identifier should print "CAN Identifier unknown"
 */
void test_TC_AEB_CTRL_X12(void)
{
    can_msg captured_frame;

    TEST_IGNORE_MESSAGE("Test case for unknown identifier");

    captured_frame.identifier = ID_EMPTY; // Unknown ID
    // The test could capture the printf output and verify the printed message.
    translateAndCallCanMsg(captured_frame);
    // TEST_ASSERT_EQUAL_STRING("CAN Identifier unknown\n", capture_stdout());
}

/**
 * @brief Test Case: Test reverse flag handling in Speed message
 */
void test_TC_AEB_CTRL_X13(void)
{
    can_msg captured_frame;

    captured_frame.identifier = ID_SPEED_S;
    captured_frame.dataFrame[0] = 0x00;
    captured_frame.dataFrame[1] = 0x64;  // Speed = 100
    captured_frame.dataFrame[2] = 0x01;  // Reverse enabled
    translateAndCallCanMsg(captured_frame);
    TEST_ASSERT_TRUE(aeb_internal_state.reverseEnabled);  // Reverse should be enabled
}

/**
 * @brief Test Case: Test the behavior when receiving clear data for speed
 */
void test_TC_AEB_CTRL_X14(void)
{
    can_msg captured_frame;

    captured_frame.dataFrame[0] = 0xFE;  // Clear data for speed
    captured_frame.dataFrame[1] = 0xFF;  
    captured_frame.dataFrame[2] = 0x00;
    translateAndCallCanMsg(captured_frame);
    TEST_ASSERT_EQUAL_FLOAT(aeb_internal_state.relative_velocity, 0.0);  // Speed should reset to 0
    TEST_ASSERT_FALSE(aeb_internal_state.reverseEnabled);  // Reverse should be disabled
}

/**
 * @brief Test Case: Test the behavior when receiving invalid data for obstacle
 */
void test_TC_AEB_CTRL_X15(void)
{
    can_msg captured_frame;

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
    // The following tests comply with [SwR-6], [SwR-9] and [SwR-11].
    RUN_TEST(test_TC_AEB_CTRL_001);
    RUN_TEST(test_TC_AEB_CTRL_002);
    RUN_TEST(test_TC_AEB_CTRL_003);
    RUN_TEST(test_TC_AEB_CTRL_004);
    
    // The following tests comply with [SwR-6], [SwR-9] and [SwR-11].
    RUN_TEST(test_TC_AEB_CTRL_005);
    RUN_TEST(test_TC_AEB_CTRL_006);
    RUN_TEST(test_TC_AEB_CTRL_007);
    RUN_TEST(test_TC_AEB_CTRL_008);
    RUN_TEST(test_TC_AEB_CTRL_009);
    RUN_TEST(test_TC_AEB_CTRL_X01);
    
    // The following tests comply with [SwR-3], [SwR-6], [SwR-7], [SwR-8], [SwR-9], [SwR-11] and [SwR-15]
    RUN_TEST(test_TC_AEB_CTRL_010);
    RUN_TEST(test_TC_AEB_CTRL_011);
    RUN_TEST(test_TC_AEB_CTRL_012);
    RUN_TEST(test_TC_AEB_CTRL_X02);
    RUN_TEST(test_TC_AEB_CTRL_X03);
    RUN_TEST(test_TC_AEB_CTRL_013);

    // The following tests comply with [SwR-2], [SwR-3], [SwR-7], [SwR-8], [SwR-11], [SwR-12] and [SwR-16]
    RUN_TEST(test_TC_AEB_CTRL_014);
    RUN_TEST(test_TC_AEB_CTRL_015);
    RUN_TEST(test_TC_AEB_CTRL_X04);
    RUN_TEST(test_TC_AEB_CTRL_X05);
    
    // The following tests comply with [SwR-2], [SwR-3], [SwR-6], [SwR-7], [SwR-8], [SwR-9], [SwR-11], 
    // [SwR-12], [SwR-15] and [SwR-16].
    RUN_TEST(test_TC_AEB_CTRL_X06);
    RUN_TEST(test_TC_AEB_CTRL_X07);
    RUN_TEST(test_TC_AEB_CTRL_X08);
    RUN_TEST(test_TC_AEB_CTRL_X09);
    RUN_TEST(test_TC_AEB_CTRL_X10);
    RUN_TEST(test_TC_AEB_CTRL_016);
    RUN_TEST(test_TC_AEB_CTRL_017);
    RUN_TEST(test_TC_AEB_CTRL_018);
    RUN_TEST(test_TC_AEB_CTRL_019);
    RUN_TEST(test_TC_AEB_CTRL_020);

    // The following tests comply with [SwR-9].
    RUN_TEST(test_TC_AEB_CTRL_021);
    RUN_TEST(test_TC_AEB_CTRL_X11);
    RUN_TEST(test_TC_AEB_CTRL_022);
    RUN_TEST(test_TC_AEB_CTRL_023);
    RUN_TEST(test_TC_AEB_CTRL_024);
    RUN_TEST(test_TC_AEB_CTRL_X12);
    RUN_TEST(test_TC_AEB_CTRL_X13);
    RUN_TEST(test_TC_AEB_CTRL_X14);
    RUN_TEST(test_TC_AEB_CTRL_X15);
    return UNITY_END();
}