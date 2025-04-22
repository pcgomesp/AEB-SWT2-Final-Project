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

//////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @note The naming convention for test cases is as follows:
 * test_<test_case_name><number_suffix>
 * 
 * Where: 
 * <test_case_name> is the name of the test case
 * 
 * <number_suffix> is a number suffix to differentiate between test cases, if begin with an "X",
 * it means that the test case is not a part of the sheet test built for check the
 * compliance with software requirements and is used for increase the coverage.
 */
//////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Helper function to check the pedal state.
 * 
 * This function verifies the internal state of the accelerator and brake pedals 
 * in the AEB system by comparing the actual state with the expected values. 
 * It asserts that the internal states of both the accelerator pedal and the brake 
 * pedal are correctly set based on the expected values.
 * 
 * @param expected_accelerator Expected state of the accelerator pedal (true for ON, false for OFF).
 * @param expected_brake Expected state of the brake pedal (true for ON, false for OFF).
 * 
 * @details
 * This helper function checks the states of the accelerator and brake pedals in 
 * the internal AEB system state (`aeb_internal_state`). It compares the actual state 
 * of each pedal with the expected state and asserts that the values match. 
 * If the expected state is `true`, the corresponding pedal should be ON; if `false`, 
 * it should be OFF.
 * 
 * @pre The internal state of the AEB system has been updated with new pedal data.
 * @post The test verifies that the internal state reflects the expected pedal states.
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
 * 
 * This test case verifies that when the accelerator pedal is ON and the brake pedal is OFF,
 * the internal state of the AEB system is updated accordingly. Specifically, the function
 * `updateInternalPedalsState` is called with the given CAN message, and the expected behavior 
 * is that the accelerator pedal state is set to ON and the brake pedal state is set to OFF.
 * 
 * @details
 * The CAN message data used in this test is:
 * - Accelerator pedal: ON (0x01)
 * - Brake pedal: OFF (0x00)
 * The test asserts that the internal AEB system state reflects these conditions correctly.
 * 
 * @pre The internal state of the AEB system is initialized.
 * @post The internal state of the AEB system reflects the changes: accelerator ON, brake OFF.
 * 
 * @anchor TC_AEB_CTRL_001
 */
void test_TC_AEB_CTRL_001(void) {
    can_msg captured_frame = { .identifier = ID_PEDALS, .dataFrame = {0x01, 0x00} };

    updateInternalPedalsState(captured_frame);

    // Test: Accelerator pedal should be ON, Brake pedal should be OFF
    checkPedalState(true, false);
}

/**
 * @brief Test Case TC_AEB_CTRL_002: Accelerator pedal ON, Brake pedal ON.
 * 
 * This test case verifies that when both the accelerator pedal and brake pedal are ON, 
 * the internal state of the AEB system is updated accordingly. Specifically, the function 
 * `updateInternalPedalsState` is called with the given CAN message, and the expected behavior 
 * is that both the accelerator pedal state and the brake pedal state are set to ON.
 * 
 * @details
 * The CAN message data used in this test is:
 * - Accelerator pedal: ON (0x01)
 * - Brake pedal: ON (0x01)
 * The test asserts that the internal AEB system state reflects these conditions correctly.
 * 
 * @pre The internal state of the AEB system is initialized.
 * @post The internal state of the AEB system reflects the changes: accelerator ON, brake ON.
 * 
 * @anchor TC_AEB_CTRL_002
 */
void test_TC_AEB_CTRL_002(void) {
    can_msg captured_frame = { .identifier = ID_PEDALS, .dataFrame = {0x01, 0x01} };

    updateInternalPedalsState(captured_frame);

    // Test: Accelerator pedal should be ON, Brake pedal should be ON
    checkPedalState(true, true);
}

/**
 * @brief Test Case TC_AEB_CTRL_003: Accelerator pedal OFF, Brake pedal OFF.
 * 
 * This test case verifies that when both the accelerator pedal and the brake pedal are OFF, 
 * the internal state of the AEB system is updated accordingly. Specifically, the function 
 * `updateInternalPedalsState` is called with the given CAN message, and the expected behavior 
 * is that both the accelerator pedal state and the brake pedal state are set to OFF.
 * 
 * @details
 * The CAN message data used in this test is:
 * - Accelerator pedal: OFF (0x00)
 * - Brake pedal: OFF (0x00)
 * The test asserts that the internal AEB system state reflects these conditions correctly.
 * 
 * @pre The internal state of the AEB system is initialized.
 * @post The internal state of the AEB system reflects the changes: accelerator OFF, brake OFF.
 * 
 * @anchor TC_AEB_CTRL_003
 */
void test_TC_AEB_CTRL_003(void) {
    can_msg captured_frame = { .identifier = ID_PEDALS, .dataFrame = {0x00, 0x00} };

    updateInternalPedalsState(captured_frame);

    // Test: Accelerator pedal should be OFF, Brake pedal should be OFF
    checkPedalState(false, false);
}

/**
 * @brief Test Case TC_AEB_CTRL_004: Accelerator pedal OFF, Brake pedal ON.
 * 
 * This test case verifies that when the accelerator pedal is OFF and the brake pedal is ON, 
 * the internal state of the AEB system is updated accordingly. Specifically, the function 
 * `updateInternalPedalsState` is called with the given CAN message, and the expected behavior 
 * is that the accelerator pedal state is set to OFF and the brake pedal state is set to ON.
 * 
 * @details
 * The CAN message data used in this test is:
 * - Accelerator pedal: OFF (0x00)
 * - Brake pedal: ON (0x01)
 * The test asserts that the internal AEB system state reflects these conditions correctly.
 * 
 * @pre The internal state of the AEB system is initialized.
 * @post The internal state of the AEB system reflects the changes: accelerator OFF, brake ON.
 * 
 * @anchor TC_AEB_CTRL_004
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
 * This function verifies the internal state of the relative velocity and the reverseEnabled 
 * flag in the AEB system by comparing the actual state with the expected values. It asserts 
 * that the internal state for both the relative velocity and the reverseEnabled flag is 
 * correctly set based on the expected values.
 * 
 * @param expected_velocity Expected relative velocity of the vehicle.
 * @param expected_reverse_enabled Expected state of reverseEnabled (true for enabled, false for disabled).
 * 
 * @details
 * This helper function compares the actual internal states of the relative velocity and 
 * reverseEnabled in the AEB system (`aeb_internal_state`) with the expected values. It checks 
 * if the reverseEnabled state matches the expected state (ON/OFF) and if the relative velocity 
 * matches the expected value. The relative velocity is verified using the `TEST_ASSERT_EQUAL_FLOAT` 
 * assertion, and the reverseEnabled state is verified using `TEST_ASSERT_TRUE` or `TEST_ASSERT_FALSE`, 
 * depending on the expected value.
 * 
 * @pre The internal state of the AEB system has been updated with the latest speed and reverse data.
 * @post The test verifies that the internal state reflects the expected relative velocity and reverseEnabled state.
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
 * 
 * This test case simulates the reception of a CAN message for speed with the 
 * data frame `{0x00, 0x64, 0x00}`. It tests the behavior of the AEB system 
 * when the speed is updated correctly, ensuring that the relative velocity 
 * is computed and stored properly based on the received CAN message.
 * 
 * The speed is calculated as `(0x64 + (0x00 << 8)) * RES_SPEED_S`, which 
 * should result in a relative velocity of `100.0`.
 * 
 * @details
 * The test simulates a scenario where the vehicle's speed is received through a CAN message, 
 * and the internal speed state is updated accordingly. The test verifies that the speed value 
 * is correctly computed and stored in the AEB system's internal state. The expected outcome 
 * is that the relative velocity should be `100.0` km/h, and the reverse flag (`reverseEnabled`) 
 * should be `false`.
 * 
 * @pre The AEB system is in an initialized state with default values.
 * @post The relative velocity in the internal state is updated to the expected value, 
 * and the reverse flag remains false.
 * 
 * @anchor TC_AEB_CTRL_005
 */
void test_TC_AEB_CTRL_005(void) {
    can_msg captured_frame = { .identifier = ID_SPEED_S, .dataFrame = {0x00, 0x64, 0x00} };

    updateInternalSpeedState(captured_frame);

    // Test: Speed should be updated correctly (0x64 + (0x00 << 8)) * RES_SPEED_S = 100.0
    checkSpeedState(100.0, false);
}

/**
 * @brief Test Case TC_AEB_CTRL_006: Case where the CAN data is set to clear data (0xFE, 0xFF).
 * 
 * This test case simulates the reception of a CAN message with the data frame 
 * `{0xFE, 0xFF, 0x00}`. It tests the behavior of the AEB system when the 
 * CAN data is set to clear data (0xFE, 0xFF), which should trigger the system 
 * to reset the speed to `0.0` and disable the reverse flag (`reverseEnabled`).
 * 
 * @details
 * The test ensures that when the CAN message with the clear data flag (`0xFE, 0xFF`) 
 * is received, the internal state of the AEB system resets the relative velocity to `0.0` 
 * and sets the reverseEnabled flag to `false`. This is essential for ensuring that the 
 * system behaves correctly when invalid or reset data is received.
 * 
 * @pre The AEB system is initialized and the internal state has a valid velocity and reverse flag.
 * @post The relative velocity in the internal state is reset to `0.0`, and the reverse flag 
 * is set to `false`, as expected when the clear data signal is detected.
 * 
 * @anchor TC_AEB_CTRL_006
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
 * 
 * This test case simulates the reception of a CAN message with the data frame 
 * `{0x00, 0x14, 0x01}`. It tests the behavior of the AEB system when the 
 * CAN data indicates reverse operation (0x01 in `dataFrame[2]`). The expected 
 * outcome is that the system should enable the reverse flag (`reverseEnabled = true`), 
 * and the speed should be calculated as `(0x00 + (0x14 << 8)) * RES_SPEED_S`, 
 * resulting in a relative velocity of `20.0`.
 * 
 * @details
 * The test ensures that when the CAN message indicates reverse operation by setting 
 * `dataFrame[2]` to `0x01`, the system correctly updates the reverseEnabled flag 
 * and calculates the relative velocity based on the data in `dataFrame[0]` and `dataFrame[1]`.
 * The speed calculation is based on the formula `(0x00 + (0x14 << 8)) * RES_SPEED_S`, 
 * which results in `20.0` km/h.
 * 
 * @pre The AEB system is initialized and ready to receive CAN messages.
 * @post The relative velocity is updated to `20.0`, and the reverse flag is enabled (`reverseEnabled = true`).
 * 
 * @anchor TC_AEB_CTRL_007
 */
void test_TC_AEB_CTRL_007(void) {
    can_msg captured_frame = { .identifier = ID_SPEED_S, .dataFrame = {0x00, 0x14, 0x01} };

    updateInternalSpeedState(captured_frame);

    // Test: Reverse should be enabled (0x00 + (0x14 << 8)) * RES_SPEED_S = 20.0
    checkSpeedState(20.0, true);
}

/**
 * @brief Test Case TC_AEB_CTRL_008: Max speed value constraint (should be capped at 251.0).
 * 
 * This test case simulates the reception of a CAN message with the data frame 
 * `{0xFF, 0xFD, 0x00}`, which corresponds to the maximum possible speed value 
 * according to the CAN protocol. The test verifies that the AEB system correctly 
 * applies a speed cap, limiting the relative velocity to a maximum of `251.0` km/h.
 * 
 * @details
 * The CAN message has the data frame `{0xFF, 0xFD, 0x00}`, where the first two 
 * bytes are used to represent the speed value in the system. The speed calculation 
 * will result in a value that exceeds the predefined maximum limit for speed. 
 * The system should apply a constraint and cap the speed at `251.0`, ensuring that 
 * the calculated relative velocity does not exceed this limit.
 * 
 * @pre The AEB system is initialized and ready to receive CAN messages.
 * @post The relative velocity is capped at `251.0`, as the calculated value exceeds the max limit.
 * 
 * @anchor TC_AEB_CTRL_008
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
 * 
 * This test case simulates the reception of a CAN message with the data frame 
 * `{0xFF, 0xFF, 0x00}`, which represents a situation where the CAN message does 
 * not require any action or change in the internal state of the AEB system. 
 * The test ensures that the system correctly handles this scenario by keeping the 
 * relative velocity at `0.0` and leaving the `reverseEnabled` state unchanged (false).
 * 
 * @details
 * The data frame `{0xFF, 0xFF, 0x00}` indicates that there is no meaningful update 
 * required for the AEB system. The system should not modify the relative velocity 
 * or enable reverse mode based on this input. The expected outcome is that the 
 * relative velocity remains at `0.0` and `reverseEnabled` remains false.
 * 
 * @pre The AEB system is initialized and ready to receive CAN messages.
 * @post The relative velocity should remain `0.0`, and `reverseEnabled` should remain false.
 * 
 * @anchor TC_AEB_CTRL_009
 */
void test_TC_AEB_CTRL_009(void) {
    can_msg captured_frame = { .identifier = ID_SPEED_S, .dataFrame = {0xFF, 0xFF, 0x00} };

    updateInternalSpeedState(captured_frame);

    // Test: Speed should be 0.0, reverseEnabled should remain false
    checkSpeedState(0.0, false);
}

/**
 * @brief Test Case: Ensure that reverseEnabled remains false for non-reverse signals.
 * 
 * This test case simulates the reception of a CAN message with the data frame 
 * `{0x00, 0x64, 0x03}`, where the third byte (dataFrame[2]) contains a non-reverse 
 * signal. The test verifies that the `reverseEnabled` flag remains false, even if 
 * the other data values indicate the vehicle's speed (100.0).
 * 
 * @details
 * The CAN data `{0x00, 0x64, 0x03}` indicates that the vehicle is moving at a speed 
 * of 100.0 km/h, but the signal in `dataFrame[2]` (with a value of `0x03`) is not a 
 * reverse signal. As a result, the system should ensure that the `reverseEnabled` 
 * flag remains false and only updates the speed as calculated from the first two 
 * bytes of the data frame.
 * 
 * @pre The AEB system is initialized and ready to receive CAN messages.
 * @post The relative velocity should be 100.0, and `reverseEnabled` should remain false.
 * 
 * @anchor TC_AEB_CTRL_X01
 */
void test_TC_AEB_CTRL_X01(void) {
    can_msg captured_frame = { .identifier = ID_SPEED_S, .dataFrame = {0x00, 0x64, 0x03} };

    updateInternalSpeedState(captured_frame);

    // Test: Reverse should not be enabled for non-reverse signals
    checkSpeedState(100.0, false);
}

/**
 * @brief Helper function to check the obstacle state.
 * 
 * This helper function checks the state of the `has_obstacle` flag and the calculated 
 * distance to the obstacle. It compares the actual internal state of the AEB system 
 * with the expected values passed as arguments.
 * 
 * @param expected_has_obstacle Expected state of the `has_obstacle` flag (true if obstacle detected, false otherwise).
 * @param expected_distance Expected distance to the obstacle (in meters).
 * 
 * @pre The `aeb_internal_state` must be initialized.
 * @post The function will assert that the `has_obstacle` flag matches the expected value 
 *       and that the `obstacle_distance` is equal to the expected distance.
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
 * 
 * This test case simulates a situation where an obstacle is detected, and the system 
 * correctly calculates the distance to the obstacle using the CAN data.
 * The expected obstacle distance is calculated as:
 * 
 * @code
 * Expected distance = (0xD0 + (0x07 << 8)) * RES_OBSTACLE = 100.0
 * @endcode
 * 
 * @details
 * The data frame `{0xD0, 0x07, 0x01}` represents an obstacle detected at a distance 
 * of 100 meters. The test ensures that the AEB system correctly calculates the distance 
 * and updates the internal state to reflect the presence of an obstacle.
 * 
 * @pre The AEB system is initialized and ready to receive CAN messages.
 * @post The `has_obstacle` flag should be true, and `obstacle_distance` should be 100.0.
 * 
 * @anchor TC_AEB_CTRL_010
 */
void test_TC_AEB_CTRL_010(void) {
    can_msg captured_frame = { .identifier = ID_OBSTACLE_S, .dataFrame = {0xD0, 0x07, 0x01} };

    updateInternalObstacleState(captured_frame);

    // Test: Expected distance = (0xD0 + (0x07 << 8)) * RES_OBSTACLE = 100.0
    checkObstacleState(true, 100.0);
}

/**
 * @brief Test Case TC_AEB_CTRL_011: No obstacle (0x00 in dataFrame[2]).
 * 
 * This test case simulates a situation where no obstacle is detected, and the system 
 * should reset the obstacle distance to the maximum value (300.0 meters).
 * 
 * @details
 * The data frame `{0xD0, 0x07, 0x00}` indicates that there is no obstacle detected (since 
 * `dataFrame[2] == 0x00`). The test ensures that the AEB system correctly handles the 
 * absence of an obstacle by setting the `has_obstacle` flag to false and the obstacle 
 * distance to the maximum value of 300.0 meters.
 * 
 * @pre The AEB system is initialized and ready to receive CAN messages.
 * @post The `has_obstacle` flag should be false, and `obstacle_distance` should be 300.0.
 * 
 * @anchor TC_AEB_CTRL_011
 */
void test_TC_AEB_CTRL_011(void) {
    can_msg captured_frame = { .identifier = ID_OBSTACLE_S, .dataFrame = {0xD0, 0x07, 0x00} };

    updateInternalObstacleState(captured_frame);

    // Test: No obstacle detected, distance should remain the same (max distance)
    checkObstacleState(false, 300.0); // Max distance when no obstacle is detected
}

/**
 * @brief Test Case TC_AEB_CTRL_012: Data reset with 0xFE, 0xFF.
 * 
 * This test case simulates a situation where the CAN data indicates a reset action, 
 * represented by the data frame `{0xFE, 0xFF, 0x00}`. When this data is received, 
 * the obstacle detection system should reset the obstacle distance to the maximum 
 * value (300.0 meters) and mark the obstacle as not detected.
 * 
 * @details
 * The data frame `{0xFE, 0xFF, 0x00}` is used to trigger the reset of the obstacle 
 * detection system. The test ensures that the system correctly handles this case by 
 * resetting the obstacle distance and setting `has_obstacle` to false.
 * 
 * @pre The AEB system is initialized and ready to receive CAN messages.
 * @post The `has_obstacle` flag should be false, and `obstacle_distance` should be 300.0.
 * 
 * @anchor TC_AEB_CTRL_012
 */
void test_TC_AEB_CTRL_012(void) {
    can_msg captured_frame = { .identifier = ID_OBSTACLE_S, .dataFrame = {0xFE, 0xFF, 0x00} };

    updateInternalObstacleState(captured_frame);

    // Test: Obstacle not detected, reset distance to max value (300.0)
    checkObstacleState(false, 300.0);
}

/**
 * @brief Test Case TC_AEB_CTRL_X02: Case where data doesn't need action (0xFF, 0xFF).
 * 
 * This test case checks the scenario where the CAN data contains `{0xFF, 0xFF, 0x00}`, 
 * which indicates that no action needs to be taken by the system. The obstacle detection 
 * system should keep the current state, and the obstacle distance should remain at the 
 * maximum value (300.0 meters) with no obstacle detected.
 * 
 * @details
 * The data frame `{0xFF, 0xFF, 0x00}` indicates that no updates are required for the 
 * obstacle detection system. The test ensures that the system does not alter its state 
 * and that the distance remains set to 300.0 meters when no obstacle is detected.
 * 
 * @pre The AEB system is initialized and ready to receive CAN messages.
 * @post The `has_obstacle` flag should be false, and `obstacle_distance` should remain 300.0.
 * 
 * @anchor TC_AEB_CTRL_X02
 */
void test_TC_AEB_CTRL_X02(void) {
    can_msg captured_frame = { .identifier = ID_OBSTACLE_S, .dataFrame = {0xFF, 0xFF, 0x00} };

    updateInternalObstacleState(captured_frame);

    // Test: Obstacle not detected, distance should be reset to 300.0
    checkObstacleState(false, 300.0);
}

/**
 * @brief Test Case TC_AEB_CTRL_X03: Calculated distance from CAN data.
 * 
 * This test case simulates a situation where the obstacle data is received from the 
 * CAN bus and the distance is correctly calculated based on the data in the frame. 
 * The data frame `{0xD0, 0x07, 0x01}` represents an obstacle detected at a calculated 
 * distance of 100 meters.
 * 
 * @details
 * The data frame `{0xD0, 0x07, 0x01}` represents an obstacle detected at a distance 
 * of 100 meters. The test ensures that the system correctly calculates the distance 
 * based on the CAN data and updates the internal state to reflect the presence of an 
 * obstacle at the correct distance.
 * 
 * @pre The AEB system is initialized and ready to receive CAN messages.
 * @post The `has_obstacle` flag should be true, and `obstacle_distance` should be 100.0.
 * 
 * @anchor TC_AEB_CTRL_X03
 */
void test_TC_AEB_CTRL_X03(void) {
    can_msg captured_frame = { .identifier = ID_OBSTACLE_S, .dataFrame = {0xD0, 0x07, 0x01} };

    updateInternalObstacleState(captured_frame);

    // Test: Obstacle detected, distance should be correctly calculated
    checkObstacleState(true, 100.0);
}

/**
 * @brief Test Case TC_AEB_CTRL_013: Maximum distance capped at 300.0.
 * 
 * This test case simulates a scenario where the CAN data exceeds the maximum allowed 
 * distance value. The system should cap the obstacle distance to 300.0 meters when 
 * the received data exceeds this limit.
 * 
 * @details
 * The test case uses the data frame `{0xFF, 0xFD, 0x00}` to simulate a situation where 
 * the obstacle distance exceeds the allowed maximum value. The expected behavior is that 
 * the system should cap the distance at 300.0 meters.
 * 
 * @pre The AEB system is initialized and ready to receive CAN messages.
 * @post The `has_obstacle` flag should be false, and the `obstacle_distance` should be capped at 300.0.
 * 
 * @anchor TC_AEB_CTRL_013
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
 * This helper function verifies whether the AEB system is turned ON or OFF by 
 * checking the `on_off_aeb_system` flag in the internal AEB state.
 * 
 * @param expected_state Expected state of the AEB system (ON/OFF).
 * 
 * @details
 * This function compares the expected state of the AEB system (ON/OFF) with the 
 * actual state held in the `aeb_internal_state.on_off_aeb_system` flag. It asserts 
 * whether the state matches the expected state.
 * 
 * @pre The AEB system's internal state must be initialized.
 * @post If the expected state matches the actual state, the test will pass.
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
 * 
 * This test case checks the functionality of the AEB system when it is turned ON. 
 * It simulates the reception of a CAN message where the `dataFrame[0]` is set to 
 * `0x01`, which indicates that the AEB system should be activated.
 * 
 * @details
 * The test case uses the data frame `{0x01}` to indicate that the AEB system is 
 * turned ON. The expected behavior is that the `on_off_aeb_system` flag in the 
 * `aeb_internal_state` should be set to `true`, indicating the system is active.
 * 
 * @pre The AEB system is initialized and ready to receive CAN messages.
 * @post The `on_off_aeb_system` flag should be set to true, indicating that the AEB system is ON.
 * 
 * @anchor TC_AEB_CTRL_014
 */
void test_TC_AEB_CTRL_014(void) {
    can_msg captured_frame = { .identifier = ID_CAR_C, .dataFrame = {0x01} };

    updateInternalCarCState(captured_frame);

    // Test: AEB system should be ON when dataFrame[0] is 0x01
    checkAEBSystemState(true);
}

/**
 * @brief Test Case TC_AEB_CTRL_015: AEB system OFF (dataFrame[0] == 0x00).
 * 
 * This test case simulates a scenario where the AEB system is turned OFF by setting 
 * `dataFrame[0]` to `0x00`. The system should deactivate the AEB system by setting 
 * the `on_off_aeb_system` flag to `false`.
 * 
 * @details
 * The test case uses the data frame `{0x00}` to indicate that the AEB system should be 
 * turned OFF. The expected behavior is that the `on_off_aeb_system` flag in the 
 * `aeb_internal_state` should be set to `false`.
 * 
 * @pre The AEB system is initialized and ready to receive CAN messages.
 * @post The `on_off_aeb_system` flag should be set to false, indicating that the AEB system is OFF.
 * 
 * @anchor TC_AEB_CTRL_015
 */
void test_TC_AEB_CTRL_015(void) {
    can_msg captured_frame = { .identifier = ID_CAR_C, .dataFrame = {0x00} };

    updateInternalCarCState(captured_frame);

    // Test: AEB system should be OFF when dataFrame[0] is 0x00
    checkAEBSystemState(false);
}

/**
 * @brief Test Case: AEB system remains OFF for any value other than 0x01.
 * 
 * This test case ensures that the AEB system remains OFF for any value other than `0x01` 
 * in the `dataFrame[0]` field. If a value other than `0x01` is received, the system should 
 * not activate the AEB system and the `on_off_aeb_system` flag should remain `false`.
 * 
 * @details
 * The test case uses an arbitrary value (`0x02`) in the `dataFrame[0]` to simulate a 
 * non-activation condition. The expected behavior is that the `on_off_aeb_system` flag 
 * remains `false`.
 * 
 * @pre The AEB system is initialized and ready to receive CAN messages.
 * @post The `on_off_aeb_system` flag should remain false for any value other than 0x01.
 * 
 * @anchor TC_AEB_CTRL_X04
 */
void test_TC_AEB_CTRL_X04(void) {
    can_msg captured_frame = { .identifier = ID_CAR_C, .dataFrame = {0x02} };  // Arbitrary value different from 0x01

    updateInternalCarCState(captured_frame);

    // Test: AEB system should remain OFF for any value other than 0x01
    checkAEBSystemState(false);
}

/**
 * @brief Test Case: AEB system turns back ON when dataFrame[0] is 0x01 again.
 * 
 * This test case ensures that the AEB system can be turned ON again when the 
 * `dataFrame[0]` is set to `0x01`. This test simulates a scenario where the AEB 
 * system is initially turned off and then re-enabled by receiving the correct 
 * `dataFrame` value.
 * 
 * @details
 * The test case uses the data frame `{0x01}` to simulate the reactivation of the AEB system. 
 * The expected behavior is that the `on_off_aeb_system` flag in the `aeb_internal_state` 
 * should be set to `true`, indicating that the AEB system is turned ON.
 * 
 * @pre The AEB system is initialized and ready to receive CAN messages.
 * @post The `on_off_aeb_system` flag should be set to true, indicating that the AEB system is back ON.
 * 
 * @anchor TC_AEB_CTRL_X05
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
 * This function compares the expected state of the AEB system with the actual state 
 * returned by the `getAEBState` function. It is used in test cases to verify that 
 * the AEB system is in the expected state.
 * 
 * @param expected_state The expected state of the AEB system, which should be one of 
 *                       the values in the `aeb_controller_state` enumeration.
 * @param state The actual state returned by `getAEBState`, which will be compared 
 *              to the expected state.
 */
void checkAEBState(aeb_controller_state expected_state, aeb_controller_state state) {
    TEST_ASSERT_EQUAL_INT(expected_state, state);
}

/**
 * @brief Test Case: AEB system active and TTC indicating alarm state.
 * 
 * This test case simulates a scenario where the AEB system is active, an obstacle 
 * is detected, and the Time to Collision (TTC) value indicates that the AEB system 
 * should enter the ALARM state. The expected behavior is that the AEB system will 
 * activate the alarm but not yet apply the brakes.
 * 
 * @details
 * The test uses the following inputs:
 * - `relative_velocity` set to the maximum speed enabled (`MAX_SPD_ENABLED`).
 * - `has_obstacle` set to true, indicating that an obstacle is detected.
 * - `obstacle_distance` set to 10.0 meters.
 * - `on_off_aeb_system` set to true, ensuring the AEB system is active.
 * - `reverseEnabled` set to false, indicating the vehicle is not in reverse.
 * 
 * The TTC value of `1.9` will be used to simulate a scenario where the AEB system 
 * should enter the ALARM state.
 * 
 * @pre The AEB system is initialized and active, and the sensor data is provided.
 * @post The AEB system should be in the ALARM state.
 * 
 * @anchor TC_AEB_CTRL_X06
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
 * 
 * This test case simulates a scenario where the AEB system is active, an obstacle 
 * is detected, but the Time to Collision (TTC) value is too low, indicating that 
 * the AEB system should apply the brakes. The expected behavior is that the AEB system 
 * should transition to the BRAKE state.
 * 
 * @details
 * The test uses the following inputs:
 * - `relative_velocity` set to the maximum speed enabled (`MAX_SPD_ENABLED`).
 * - `has_obstacle` set to true, indicating that an obstacle is detected.
 * - `obstacle_distance` set to 10.0 meters.
 * - `on_off_aeb_system` set to true, ensuring the AEB system is active.
 * - `reverseEnabled` set to false, indicating the vehicle is not in reverse.
 * 
 * The TTC value of `0.9` is used to simulate a scenario where the TTC is too low 
 * to avoid a collision, so the AEB system should go into the BRAKE state.
 * 
 * @pre The AEB system is initialized and active, and the sensor data is provided.
 * @post The AEB system should be in the BRAKE state.
 * 
 * @anchor TC_AEB_CTRL_X07
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
 * 
 * This test case simulates a scenario where the AEB system is turned OFF, 
 * and the vehicle is traveling at high speed. The expected behavior is that 
 * the AEB system will not engage, and the system should remain in the STANDBY state.
 * 
 * @details
 * The test uses the following inputs:
 * - `relative_velocity` set to 80.0 (high speed).
 * - `has_obstacle` set to true, indicating that an obstacle is detected.
 * - `obstacle_distance` set to 10.0 meters.
 * - `on_off_aeb_system` set to false, deactivating the AEB system.
 * - `reverseEnabled` set to false, indicating the vehicle is not in reverse.
 * 
 * The TTC value of `1.9` will be used to simulate a situation where the AEB system is inactive, 
 * and the system should remain in the STANDBY state.
 * 
 * @pre The AEB system is initialized with the specified values and the sensor data is provided.
 * @post The AEB system should be in the STANDBY state.
 * 
 * @anchor TC_AEB_CTRL_X08
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
 * 
 * This test case simulates a scenario where the vehicle's speed is below the minimum 
 * allowed speed (`MIN_SPD_ENABLED`), and the AEB system is active but the vehicle is 
 * not in reverse. The expected behavior is that the AEB system will enter the STANDBY state.
 * 
 * @details
 * The test uses the following inputs:
 * - `relative_velocity` set to a value below `MIN_SPD_ENABLED` (speed below the minimum).
 * - `has_obstacle` set to true, indicating that an obstacle is detected.
 * - `obstacle_distance` set to 10.0 meters.
 * - `on_off_aeb_system` set to true, ensuring the AEB system is active.
 * - `reverseEnabled` set to false, indicating the vehicle is not in reverse.
 * 
 * The TTC value of `1.9` is used to simulate a situation where the vehicle is moving at low speed, 
 * so the AEB system should go into the STANDBY state.
 * 
 * @pre The AEB system is initialized and active, with sensor data provided.
 * @post The AEB system should be in the STANDBY state due to low speed.
 * 
 * @anchor TC_AEB_CTRL_X09
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
 * 
 * This test case simulates a scenario where the vehicle's speed is above the maximum 
 * allowed speed (`MAX_SPD_ENABLED`), and the AEB system is active but the vehicle is 
 * not in reverse. The expected behavior is that the AEB system will enter the STANDBY state.
 * 
 * @details
 * The test uses the following inputs:
 * - `relative_velocity` set to a value above `MAX_SPD_ENABLED` (speed above the maximum).
 * - `has_obstacle` set to true, indicating that an obstacle is detected.
 * - `obstacle_distance` set to 10.0 meters.
 * - `on_off_aeb_system` set to true, ensuring the AEB system is active.
 * - `reverseEnabled` set to false, indicating the vehicle is not in reverse.
 * 
 * The TTC value of `1.9` is used to simulate a situation where the vehicle is moving at a speed 
 * above the allowed limit, so the AEB system should go into the STANDBY state.
 * 
 * @pre The AEB system is initialized and active, with sensor data provided.
 * @post The AEB system should be in the STANDBY state due to high speed.
 * 
 * @anchor TC_AEB_CTRL_X10
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
 * 
 * This test case simulates a situation where the accelerator and brake pedals are deactivated, 
 * and the Time to Collision (TTC) is below the braking threshold. The expected behavior is that 
 * the AEB system will activate the brakes to avoid a potential collision.
 * 
 * @details
 * The test uses the following inputs:
 * - `relative_velocity` set to 60.0 (normal driving speed).
 * - `has_obstacle` set to true, indicating that an obstacle is detected.
 * - `obstacle_distance` set to 10.0 meters.
 * - `on_off_aeb_system` set to true, activating the AEB system.
 * - `reverseEnabled` set to false, indicating that the vehicle is not in reverse.
 * - `accelerator_pedal` set to false and `brake_pedal` set to false, simulating deactivated pedals.
 * - `ttc` set to 0.8, which is lower than the braking threshold.
 * 
 * The AEB system should transition to the BRAKE state because the TTC value is below the threshold.
 * 
 * @pre The AEB system is active, and sensor data is provided with low TTC.
 * @post The AEB system should be in the BRAKE state to initiate braking.
 * 
 * @anchor TC_AEB_CTRL_016
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
 * 
 * This test case simulates a situation where the accelerator and brake pedals are deactivated, 
 * and the Time to Collision (TTC) is within the alarm range. The expected behavior is that the 
 * AEB system will activate the alarm to warn the driver of a potential collision.
 * 
 * @details
 * The test uses the following inputs:
 * - `relative_velocity` set to 60.0 (normal driving speed).
 * - `has_obstacle` set to true, indicating that an obstacle is detected.
 * - `obstacle_distance` set to 10.0 meters.
 * - `on_off_aeb_system` set to true, activating the AEB system.
 * - `reverseEnabled` set to false, indicating that the vehicle is not in reverse.
 * - `accelerator_pedal` set to false and `brake_pedal` set to false, simulating deactivated pedals.
 * - `ttc` set to 1.5, which is greater than the braking threshold but less than the alarm threshold.
 * 
 * The AEB system should transition to the ALARM state because the TTC value is within the alarm range.
 * 
 * @pre The AEB system is active, and sensor data is provided with TTC within alarm range.
 * @post The AEB system should be in the ALARM state to activate the warning system.
 * 
 * @anchor TC_AEB_CTRL_017
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
 * 
 * This test case simulates a situation where the accelerator and brake pedals are deactivated, 
 * and the Time to Collision (TTC) is greater than the alarm threshold. The expected behavior is that 
 * the AEB system will remain in the ACTIVE state, indicating normal operation.
 * 
 * @details
 * The test uses the following inputs:
 * - `relative_velocity` set to 60.0 (normal driving speed).
 * - `has_obstacle` set to true, indicating that an obstacle is detected.
 * - `obstacle_distance` set to 10.0 meters.
 * - `on_off_aeb_system` set to true, activating the AEB system.
 * - `reverseEnabled` set to false, indicating that the vehicle is not in reverse.
 * - `accelerator_pedal` set to false and `brake_pedal` set to false, simulating deactivated pedals.
 * - `ttc` set to `THRESHOLD_ALARM + 0.1`, which is greater than the alarm threshold.
 * 
 * The AEB system should remain in the ACTIVE state because the TTC value is above the alarm threshold.
 * 
 * @pre The AEB system is active, and sensor data is provided with TTC greater than the alarm threshold.
 * @post The AEB system should remain in the ACTIVE state.
 * 
 * @anchor TC_AEB_CTRL_018
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
 * 
 * This test case simulates a situation where the brake pedal is pressed while the accelerator 
 * pedal is off. Despite the presence of an obstacle and a non-zero Time to Collision (TTC), 
 * the AEB system should go to the ACTIVE state because the brake pedal is pressed, which indicates 
 * an intentional action by the driver.
 * 
 * @details
 * The test uses the following inputs:
 * - `relative_velocity` set to 60.0 (normal driving speed).
 * - `has_obstacle` set to true, indicating that an obstacle is detected.
 * - `obstacle_distance` set to 10.0 meters.
 * - `on_off_aeb_system` set to true, activating the AEB system.
 * - `reverseEnabled` set to false, indicating that the vehicle is not in reverse.
 * - `accelerator_pedal` set to false, indicating that the accelerator is off.
 * - `brake_pedal` set to true, indicating that the brake pedal is pressed.
 * - `ttc` set to 1.5, which is within the range where the AEB system would normally consider the 
 *   possibility of braking or alarming.
 * 
 * Regardless of the TTC value, the AEB system should transition to the ACTIVE state because the brake 
 * pedal is pressed.
 * 
 * @pre The AEB system is active, and the brake pedal is pressed.
 * @post The AEB system should be in the ACTIVE state, indicating normal operation with the brake pedal engaged.
 * 
 * @anchor TC_AEB_CTRL_019
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
 * 
 * This test case simulates a situation where the accelerator pedal is pressed while the brake 
 * pedal is off. The AEB system should go to the ACTIVE state as the accelerator pedal is engaged 
 * and there is no immediate need for braking (based on the TTC value).
 * 
 * @details
 * The test uses the following inputs:
 * - `relative_velocity` set to 60.0 (normal driving speed).
 * - `has_obstacle` set to true, indicating that an obstacle is detected.
 * - `obstacle_distance` set to 10.0 meters.
 * - `on_off_aeb_system` set to true, activating the AEB system.
 * - `reverseEnabled` set to false, indicating that the vehicle is not in reverse.
 * - `accelerator_pedal` set to true, indicating that the accelerator is pressed.
 * - `brake_pedal` set to false, indicating that the brake pedal is not pressed.
 * - `ttc` set to 1.5, which is within the range where the AEB system would normally consider the 
 *   possibility of braking or alarming.
 * 
 * Since the accelerator pedal is engaged and no critical braking situation is present (based on TTC), 
 * the AEB system should remain in the ACTIVE state, continuing normal operation.
 * 
 * @pre The AEB system is active, and the accelerator pedal is pressed.
 * @post The AEB system should be in the ACTIVE state, indicating normal operation with the accelerator pedal engaged.
 * 
 * @anchor TC_AEB_CTRL_020
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
 * 
 * This test case ensures that the `updateInternalPedalsState` function correctly handles the CAN 
 * messages related to the pedal states. The function should correctly update the internal state of 
 * the accelerator and brake pedals based on the values received in the CAN message.
 * 
 * @details
 * The test uses the following inputs:
 * - A CAN message with the identifier `ID_PEDALS` that sets the accelerator pedal to ON (`0x01`) 
 *   and the brake pedal to OFF (`0x00`).
 * 
 * The expected result is that the `accelerator_pedal` state will be set to true (ON), and the 
 * `brake_pedal` state will be set to false (OFF) in the `aeb_internal_state`.
 * 
 * @pre The CAN message has been received with the correct data for the pedals.
 * @post The internal state of the accelerator and brake pedals should be updated correctly.
 * 
 * @anchor TC_AEB_CTRL_021
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
 * 
 * This test case ensures that when the CAN message with the `ID_PEDALS` identifier is received, 
 * the `updateInternalPedalsState` function correctly updates the state of the pedals in the internal 
 * AEB system. Specifically, when the brake pedal is ON (`0x01`) and the accelerator is OFF (`0x00`), 
 * the internal state should reflect these values.
 * 
 * @details
 * The test uses the following inputs:
 * - `identifier` set to `ID_PEDALS`, which indicates a message related to the pedals.
 * - `dataFrame[0]` set to `0x00`, indicating that the accelerator pedal is OFF.
 * - `dataFrame[1]` set to `0x01`, indicating that the brake pedal is ON.
 * 
 * The expected result is that:
 * - The `accelerator_pedal` in the internal state should be OFF (false).
 * - The `brake_pedal` in the internal state should be ON (true).
 * 
 * @pre The CAN message is correctly formatted with `ID_PEDALS` and pedal states.
 * @post The internal state of the accelerator and brake pedals should be updated as expected.
 * 
 * @anchor TC_AEB_CTRL_X11
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
 * 
 * This test case verifies that the `updateInternalSpeedState` function is correctly called and updates 
 * the internal state of the vehicle's relative velocity when a CAN message with the `ID_SPEED_S` identifier 
 * is received. The message should correctly reflect the vehicle's speed in km/h, based on the data in 
 * `dataFrame[0]` and `dataFrame[1]`.
 * 
 * @details
 * The test uses the following inputs:
 * - `identifier` set to `ID_SPEED_S`, indicating a message related to the vehicle's speed.
 * - `dataFrame[0]` set to `0x00`, and `dataFrame[1]` set to `0x64`, which together represent the value `0x0064` (100 in decimal) for the vehicle's speed.
 * - `dataFrame[2]` set to `0x00`, which is not used for this test case.
 * 
 * The expected result is that the `relative_velocity` in the internal state is updated to `100.0`, which 
 * corresponds to the vehicle's speed in km/h.
 * 
 * @pre The CAN message is correctly formatted with `ID_SPEED_S` and the correct speed data.
 * @post The internal state `relative_velocity` should be updated to 100.0 km/h.
 * 
 * @anchor TC_AEB_CTRL_022
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
 * 
 * This test case ensures that when the CAN message with the `ID_OBSTACLE_S` identifier is received, 
 * the `updateInternalObstacleState` function correctly updates the internal state of the AEB system with 
 * the presence of an obstacle and the corresponding distance to it.
 * 
 * @details
 * The test uses the following inputs:
 * - `identifier` set to `ID_OBSTACLE_S`, indicating a message related to obstacle detection.
 * - `dataFrame[0]` set to `0xD0`, and `dataFrame[1]` set to `0x07`, together representing the obstacle distance of 100 meters.
 * - `dataFrame[2]` set to `0x01`, indicating that an obstacle is detected.
 * 
 * The expected result is that:
 * - The `has_obstacle` in the internal state should be true, indicating that an obstacle is detected.
 * - The `obstacle_distance` in the internal state should be updated to `100.0` meters.
 * 
 * @pre The CAN message is correctly formatted with `ID_OBSTACLE_S` and obstacle data.
 * @post The internal state `has_obstacle` should be true, and `obstacle_distance` should be 100.0 meters.
 * 
 * @anchor TC_AEB_CTRL_023
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
 * 
 * This test case verifies that when a CAN message with the `ID_CAR_C` identifier is received, 
 * the `updateInternalCarCState` function correctly updates the internal state of the AEB system, 
 * specifically turning the AEB system ON or OFF based on the received data.
 * 
 * @details
 * The test uses the following inputs:
 * - `identifier` set to `ID_CAR_C`, indicating a message related to the AEB system state.
 * - `dataFrame[0]` set to `0x01`, indicating that the AEB system should be ON.
 * 
 * The expected result is that:
 * - The `on_off_aeb_system` in the internal state should be set to `true`, indicating that the AEB system is ON.
 * 
 * @pre The CAN message is correctly formatted with `ID_CAR_C` and the AEB system state.
 * @post The internal state `on_off_aeb_system` should be updated to `true` (AEB system ON).
 * 
 * @anchor TC_AEB_CTRL_024
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
 * 
 * This test case verifies that when a CAN message with an unknown identifier is received, 
 * the system correctly handles the invalid identifier by printing the message "CAN Identifier unknown".
 * 
 * @details
 * The test uses the following inputs:
 * - `identifier` set to `ID_EMPTY`, which is an unknown identifier that is not recognized by the system.
 * 
 * The expected result is that the system should print the message "CAN Identifier unknown" to indicate 
 * that the received identifier is invalid.
 * 
 * @pre The CAN message is correctly formatted with `ID_EMPTY`, representing an unknown identifier.
 * @post The system should print "CAN Identifier unknown" to handle the unrecognized identifier.
 * 
 * @note This test currently ignores the check for printed output. It can be modified to capture 
 *       and verify the printed output if necessary.
 * 
 * @anchor TC_AEB_CTRL_X12
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
 * 
 * This test case verifies that when the CAN message with the `ID_SPEED_S` identifier is received 
 * and includes the reverse flag, the internal state correctly updates the `reverseEnabled` flag 
 * based on the value in `dataFrame[2]`.
 * 
 * @details
 * The test uses the following inputs:
 * - `identifier` set to `ID_SPEED_S`, indicating a message related to the vehicle's speed.
 * - `dataFrame[0]` set to `0x00` and `dataFrame[1]` set to `0x64`, indicating a speed of 100 km/h.
 * - `dataFrame[2]` set to `0x01`, indicating that reverse is enabled.
 * 
 * The expected result is that:
 * - The `reverseEnabled` in the internal state should be set to `true`, indicating that the vehicle is in reverse.
 * 
 * @pre The CAN message is correctly formatted with `ID_SPEED_S` and the reverse flag data.
 * @post The internal state `reverseEnabled` should be updated to `true`.
 * 
 * @anchor TC_AEB_CTRL_X13
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
 * 
 * This test case verifies the behavior of the system when receiving a CAN message 
 * that clears the speed data. The `dataFrame` values are set to `0xFE` and `0xFF`, 
 * which should indicate that the speed data should be reset.
 * 
 * @details
 * The test uses the following inputs:
 * - `dataFrame[0]` set to `0xFE`, which indicates that the speed data should be cleared.
 * - `dataFrame[1]` set to `0xFF` and `dataFrame[2]` set to `0x00`, which are used as part of the clear command.
 * 
 * The expected result is that:
 * - The internal state `relative_velocity` should be reset to `0.0`.
 * - The internal state `reverseEnabled` should be set to `false`, as no reverse signal is provided.
 * 
 * @pre The CAN message is correctly formatted with the clear data command for speed.
 * @post The internal state `relative_velocity` should be set to `0.0`, and `reverseEnabled` should be `false`.
 * 
 * @anchor TC_AEB_CTRL_X14
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
 * 
 * This test case verifies the behavior of the system when receiving a CAN message 
 * with invalid data for the obstacle. The data is set to `0xFF`, indicating that 
 * no obstacle is detected.
 * 
 * @details
 * The test uses the following inputs:
 * - `identifier` set to `ID_OBSTACLE_S`, indicating a message related to obstacle detection.
 * - `dataFrame[0]` and `dataFrame[1]` are set to `0xFF`, which represent invalid or no obstacle data.
 * - `dataFrame[2]` is set to `0xFF`, signaling that no obstacle is detected.
 * 
 * The expected result is that:
 * - The internal state `has_obstacle` should be set to `false`, indicating no obstacle is detected.
 * - The internal state `obstacle_distance` should be reset to `0.0`, as no valid obstacle data is provided.
 * 
 * @pre The CAN message is correctly formatted with invalid obstacle data.
 * @post The internal state `has_obstacle` should be set to `false`, and `obstacle_distance` should be set to `0.0`.
 * 
 * @anchor TC_AEB_CTRL_X15
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
 * @brief Helper function to check the CAN message output for specific warning and brake values.
 * 
 * This function verifies that the CAN message contains the expected values for the 
 * warning system and braking system based on the provided parameters.
 * 
 * @param expected_warning Expected state for the warning system (0x00 or 0x01).
 * @param expected_brake Expected state for the braking system (0x00 or 0x01).
 * @param msg The actual CAN message to be checked.
 * 
 * @pre The `msg` is a valid CAN message with the expected values for the warning and braking system.
 * @post The function checks if the warning and braking system values match the expected values.
 */
void checkCanMsgOutput(int expected_warning, int expected_brake, can_msg msg) {
    TEST_ASSERT_EQUAL_HEX8(expected_warning, msg.dataFrame[0]);
    TEST_ASSERT_EQUAL_HEX8(expected_brake, msg.dataFrame[1]);
}

/**
 * @brief Test for the AEB_STATE_BRAKE state in the CAN message output.
 * 
 * This test case verifies that when the AEB system is in the BRAKE state, 
 * the corresponding CAN message values for the warning system and braking system
 * are set correctly.
 * 
 * @details
 * In this case, the AEB system is in the BRAKE state, which indicates that
 * the braking system should be activated while the warning system is also
 * triggered. The expected values in the CAN message are:
 * - `dataFrame[0]` should be `0x01` to activate the warning system.
 * - `dataFrame[1]` should be `0x01` to activate the braking system.
 * 
 * @pre The AEB system state is set to `AEB_STATE_BRAKE`.
 * @post The CAN message will have the warning system and braking system set to `0x01`.
 * 
 * @anchor TC_AEB_CTRL_X16
 */
void test_TC_AEB_CTRL_X16(void) {
    aeb_controller_state state = AEB_STATE_BRAKE;
    can_msg result = updateCanMsgOutput(state);

    checkCanMsgOutput(0x01, 0x01, result);
}

/**
 * @brief Test for the AEB_STATE_ALARM state in the CAN message output.
 * 
 * This test case verifies that when the AEB system is in the ALARM state, 
 * the corresponding CAN message values for the warning system and braking system
 * are set correctly.
 * 
 * @details
 * In this case, the AEB system is in the ALARM state, which indicates that
 * a potential collision has been detected, and the warning system is activated.
 * However, the braking system should not be activated yet. The expected values in
 * the CAN message are:
 * - `dataFrame[0]` should be `0x01` to activate the warning system.
 * - `dataFrame[1]` should be `0x00` to deactivate the braking system.
 * 
 * @pre The AEB system state is set to `AEB_STATE_ALARM`.
 * @post The CAN message will have the warning system activated and the braking system deactivated.
 * 
 * @anchor TC_AEB_CTRL_X17
 */
void test_TC_AEB_CTRL_X17(void) {
    aeb_controller_state state = AEB_STATE_ALARM;
    can_msg result = updateCanMsgOutput(state);

    checkCanMsgOutput(0x01, 0x00, result);
}

/**
 * @brief Test for the AEB_STATE_ACTIVE state in the CAN message output.
 * 
 * This test case verifies that when the AEB system is in the ACTIVE state, 
 * the corresponding CAN message values for the warning system and braking system
 * are set correctly.
 * 
 * @details
 * In this case, the AEB system is in the ACTIVE state, which means that the
 * system is not in alarm and the warning system and braking system should
 * be deactivated. The expected values in the CAN message are:
 * - `dataFrame[0]` should be `0x00` to deactivate the warning system.
 * - `dataFrame[1]` should be `0x00` to deactivate the braking system.
 * 
 * @pre The AEB system state is set to `AEB_STATE_ACTIVE`.
 * @post The CAN message will have both the warning system and braking system deactivated (`0x00`).
 * 
 * @anchor TC_AEB_CTRL_X18
 */
void test_TC_AEB_CTRL_X18(void) {
    aeb_controller_state state = AEB_STATE_ACTIVE;
    can_msg result = updateCanMsgOutput(state);

    checkCanMsgOutput(0x00, 0x00, result);
}

/**
 * @brief Test for the AEB_STATE_STANDBY state in the CAN message output.
 * 
 * This test verifies that when the AEB system is in the STANDBY state, 
 * the corresponding CAN message values for the warning system and braking system
 * are set correctly.
 * 
 * @details
 * In this case, the AEB system is in the STANDBY state, which indicates that
 * the system is idle and not actively engaged in monitoring or braking. 
 * Both the warning system and the braking system should be inactive.
 * The expected values in the CAN message are:
 * - `dataFrame[0]` should be `0x00` to deactivate the warning system.
 * - `dataFrame[1]` should be `0x00` to deactivate the braking system.
 * 
 * @pre The AEB system state is set to `AEB_STATE_STANDBY`.
 * @post The CAN message will have both the warning system and braking system deactivated (`0x00`).
 * 
 * @anchor TC_AEB_CTRL_X19
 */
void test_TC_AEB_CTRL_X19(void) {
    aeb_controller_state state = AEB_STATE_STANDBY;
    can_msg result = updateCanMsgOutput(state);

    checkCanMsgOutput(0x00, 0x00, result);
}

/**
 * @brief Test for an invalid AEB system state (out of the defined enumeration range).
 * 
 * This test verifies that when an invalid AEB state (e.g., 999) is provided,
 * the corresponding CAN message retains the default values.
 * 
 * @details
 * This test simulates the scenario where an invalid or undefined AEB state 
 * is provided to the system. In such cases, the system should not change the 
 * default values of the CAN message. The expected values in the CAN message 
 * are the default values defined in the system, which are:
 * - `dataFrame[0]` should be `0xFF` (default value).
 * - `dataFrame[1]` should be `0xFF` (default value).
 * 
 * @pre The AEB system state is set to an invalid value (e.g., `999`).
 * @post The CAN message will retain the default values (`0xFF`).
 * 
 * @anchor TC_AEB_CTRL_X20
 */
void test_TC_AEB_CTRL_X20(void) {
    aeb_controller_state state = (aeb_controller_state)999; // Invalid value
    can_msg result = updateCanMsgOutput(state);

    checkCanMsgOutput(0xFF, 0xFF, result);
}

/**
 * @brief Test for valid and invalid pedal states.
 * 
 * This test verifies that the internal pedal states (accelerator and brake) are correctly 
 * updated based on the received CAN message. The test checks both valid and invalid input 
 * values for the pedal states.
 * 
 * @details
 * This test simulates two scenarios:
 * 1. When the received CAN message contains valid data (`0x01` for both accelerator and 
 *    brake pedals), both pedals should be turned ON.
 * 2. When the received CAN message contains invalid data (`0x02` for both pedals), the 
 *    internal pedal states should remain unchanged (still ON), as the data is not processed 
 *    as valid.
 * 
 * @pre The CAN message is initialized with valid data (`0x01` for both pedals) and then 
 *      modified to invalid data (`0x02` for both pedals).
 * @post The internal pedal states will reflect the expected values for valid input and 
 *       remain unchanged for invalid input.
 * 
 * @anchor TC_AEB_CTRL_X21
 */
void test_TC_AEB_CTRL_X21(void) {
    can_msg captured_frame = { .identifier = ID_PEDALS, .dataFrame = {0x01, 0x01} };

    updateInternalPedalsState(captured_frame);

    // Test: Accelerator pedal should be ON, Brake pedal should be ON
    checkPedalState(true, true);

    captured_frame.dataFrame[0] = 0x02;
    captured_frame.dataFrame[1] = 0x02;
    updateInternalPedalsState(captured_frame);

    // Test: Accelerator and Brake Pedal should be on yet, since this dataFrame isn't valid
    checkPedalState(true, true);
}

/**
 * @brief Test Case: Update internal obstacle state with specific CAN data.
 * 
 * This test case verifies the behavior when a CAN message with the identifier
 * `ID_OBSTACLE_S` is received with specific data values that should update the
 * internal obstacle state. The data frame `{0xFE, 0xFF, 0x01}` is used to
 * simulate a scenario where an obstacle is detected at a preset distance.
 * 
 * @details
 * This test ensures that the `updateInternalObstacleState` function handles 
 * the given data correctly, updating the internal state to indicate an obstacle 
 * is present. The test checks that the obstacle detection flag is set to `true` 
 * and the obstacle distance is maintained at 300.0 meters.
 * 
 * @pre The AEB system is initialized and ready to process CAN messages.
 * @post The internal state should reflect the presence of an obstacle with 
 * a distance of 300.0 meters.
 * 
 * @anchor TC_AEB_CTRL_X22
 */

void test_TC_AEB_CTRL_X22()
{
    can_msg captured_frame = { .identifier = ID_OBSTACLE_S, .dataFrame = {0xFE, 0xFF, 0x01} };
    updateInternalObstacleState(captured_frame);
    checkObstacleState(true, 300.0);
}

/**
 * @brief Test Case: Update internal obstacle state with specific CAN data.
 * 
 * This test case verifies the behavior when a CAN message with the identifier
 * `ID_OBSTACLE_S` is received with specific data values that should update the
 * internal obstacle state. The data frame `{0xFD, 0xFF, 0x01}` is used to
 * simulate a scenario where an obstacle is detected at a preset distance.
 * 
 * @details
 * This test ensures that the `updateInternalObstacleState` function handles 
 * the given data correctly, updating the internal state to indicate an obstacle 
 * is present. The test checks that the obstacle detection flag is set to `true` 
 * and the obstacle distance is maintained at 300.0 meters.
 * 
 * @pre The AEB system is initialized and ready to process CAN messages.
 * @post The internal state should reflect the presence of an obstacle with 
 * a distance of 300.0 meters.
 * 
 * @anchor TC_AEB_CTRL_X23
 */
void test_TC_AEB_CTRL_X23()
{
    can_msg captured_frame = { .identifier = ID_OBSTACLE_S, .dataFrame = {0xFD, 0xFF, 0x01} };
    updateInternalObstacleState(captured_frame);
    checkObstacleState(true, 300.0);
}

/**
 * @brief Test Case: Clear acceleration data (0xFE, 0xFF).
 * 
 * @details This test verifies the behavior of the system when the acceleration
 *          portion of the CAN frame is set to the "clear data" values.
 * 
 * @pre The input CAN frame must have dataFrame[3] == 0xFE and dataFrame[4] == 0xFF.
 * 
 * @post The internal acceleration value (aeb_internal_state.relative_acceleration) 
 *       is expected to be reset to 0.0.
 * 
 * @anchor TC_AEB_CTRL_X24
 */
void test_TC_AEB_CTRL_X24(void) {
    can_msg frame = {.dataFrame = {0x00, 0x00, 0x00, 0xFE, 0xFF}};
    updateInternalSpeedState(frame);
    TEST_ASSERT_EQUAL_FLOAT(0.0, aeb_internal_state.relative_acceleration);
}

/**
 * @brief Test Case: Acceleration update skipped when set to ignore (0xFF, 0xFF).
 * 
 * @details This test ensures that the system does not update the internal acceleration value
 *          when the acceleration bytes in the CAN frame are set to 0xFF, 0xFF — a condition
 *          defined to indicate "ignore update."
 * 
 * @pre The CAN message must have dataFrame[3] == 0xFF and dataFrame[4] == 0xFF.
 * 
 * @post The internal acceleration value should remain at the default (0.0).
 * 
 * @anchor TC_AEB_CTRL_X25
 */
void test_TC_AEB_CTRL_X25(void) {
    can_msg frame = {.dataFrame = {0x00, 0x00, 0x00, 0xFF, 0xFF}};
    updateInternalSpeedState(frame);
    TEST_ASSERT_EQUAL_FLOAT(0.0, aeb_internal_state.relative_acceleration);
}

/**
 * @brief Test Case: Maximum allowed acceleration (positive).
 * 
 * @details This test verifies that the computed acceleration is capped at the maximum
 *          limit (12.5 m/s²) when the raw CAN value would exceed it.
 * 
 * @pre The acceleration bytes must decode to a value greater than MAX_ACCELERATION_S.
 * 
 * @post The stored internal acceleration should be limited to 12.5.
 * 
 * @anchor TC_AEB_CTRL_X26
 */
void test_TC_AEB_CTRL_X26(void) {
    can_msg frame = {
        .identifier = ID_SPEED_S,
        .dataFrame = {0x00, 0x00, 0x00, 0xA8, 0x61, 0x00}  // 25000 → (25000 - 12500) * 0.001 = 12.5
    };
    updateInternalSpeedState(frame);
    TEST_ASSERT_EQUAL_FLOAT(12.5, aeb_internal_state.relative_acceleration);
}

/**
 * @brief Test Case: Maximum allowed deceleration (negative).
 * 
 * @details This test validates that the system handles negative acceleration properly 
 *          using the direction flag, and ensures it respects the -12.5 m/s² minimum limit.
 * 
 * @pre CAN frame must include a direction flag set to reverse (dataFrame[5] == 0x01).
 * 
 * @post The internal acceleration should be capped at -12.5.
 * 
 * @anchor TC_AEB_CTRL_X27
 */
void test_TC_AEB_CTRL_X27(void) {
    can_msg frame = {
        .identifier = ID_SPEED_S,
        .dataFrame = {0x00, 0x00, 0x00, 0xA8, 0x61, 0x01}  // direction = reverse
    };
    updateInternalSpeedState(frame);
    TEST_ASSERT_EQUAL_FLOAT(-12.5, aeb_internal_state.relative_acceleration);
}

/**
 * @brief Test Case: Acceleration exceeding max limit is clamped.
 * 
 * @details This test ensures that even when the decoded value is above the valid maximum,
 *          the internal system clamps it to 12.5 m/s².
 * 
 * @pre Acceleration bytes must convert to a float above 12.5.
 * 
 * @post Acceleration value stored must not exceed 12.5.
 * 
 * @anchor TC_AEB_CTRL_X28
 */
void test_TC_AEB_CTRL_X28(void) {
    can_msg frame = {
        .identifier = ID_SPEED_S,
        .dataFrame = {0x00, 0x00, 0x00, 0x30, 0x75, 0x00}  // 30000 → (30000 - 12500) * 0.001 = 17.5 → clamp to 12.5
    };
    updateInternalSpeedState(frame);
    TEST_ASSERT_EQUAL_FLOAT(12.5, aeb_internal_state.relative_acceleration);
}

/**
 * @brief Test Case: Acceleration below min limit is clamped (reverse direction).
 * 
 * @details This test verifies that when a negative acceleration value exceeds the minimum
 *          allowed threshold, it is clamped to -12.5 m/s².
 * 
 * @pre The raw decoded acceleration must be below -12.5 and the direction flag must be active.
 * 
 * @post The internal acceleration is limited to -12.5.
 * 
 * @anchor TC_AEB_CTRL_X29
 */
void test_TC_AEB_CTRL_X29(void) {
    can_msg frame = {
        .identifier = ID_SPEED_S,
        .dataFrame = {0x00, 0x00, 0x00, 0x00, 0x00, 0x01}  // raw = 0 → (-12500) * 0.001 = -12.5
    };
    updateInternalSpeedState(frame);
    TEST_ASSERT_EQUAL_FLOAT(-12.5, aeb_internal_state.relative_acceleration);
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

    // Tests to verify the Output of CAN Messages
    RUN_TEST(test_TC_AEB_CTRL_X16);
    RUN_TEST(test_TC_AEB_CTRL_X17);
    RUN_TEST(test_TC_AEB_CTRL_X18);
    RUN_TEST(test_TC_AEB_CTRL_X19);
    RUN_TEST(test_TC_AEB_CTRL_X20);

    // Tests for enhancements of MC/DC coverage
    RUN_TEST(test_TC_AEB_CTRL_X21);
    RUN_TEST(test_TC_AEB_CTRL_X22);
    RUN_TEST(test_TC_AEB_CTRL_X23);

    // Tests for acceleration and deceleration
    RUN_TEST(test_TC_AEB_CTRL_X24);
    RUN_TEST(test_TC_AEB_CTRL_X25);
    RUN_TEST(test_TC_AEB_CTRL_X26);
    RUN_TEST(test_TC_AEB_CTRL_X27);
    RUN_TEST(test_TC_AEB_CTRL_X28);
    RUN_TEST(test_TC_AEB_CTRL_X29);
    return UNITY_END();
}