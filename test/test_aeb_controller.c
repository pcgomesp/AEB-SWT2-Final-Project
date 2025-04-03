#include "unity.h"
#include "constants.h"
#include "sensors_input.h"
#include "dbc.h"
#include <mqueue.h>
#include "ttc_control.h"

// Declaration of AEB states
typedef enum {
    AEB_STATE_ACTIVE,
    AEB_STATE_ALARM,
    AEB_STATE_BRAKE,
    AEB_STATE_STANDBY
} aeb_controller_state;

// External declaration of global variables
extern sensors_input_data aeb_internal_state;
extern can_msg captured_can_frame;
extern can_msg out_can_frame;
extern can_msg empty_msg;

// Declaration of functions implemented in aeb_controller.c that will be tested
void translateAndCallCanMsg(can_msg captured_frame);
void updateInternalPedalsState(can_msg captured_frame);
void updateInternalSpeedState(can_msg captured_frame);
void updateInternalObstacleState(can_msg captured_frame);
void updateInternalCarCState(can_msg captured_frame);
can_msg updateCanMsgOutput(aeb_controller_state state);
aeb_controller_state getAEBState(sensors_input_data aeb_internal_state, double ttc);

// Mock functions
// Mock for open_mq
mqd_t open_mq(char *mq_name) {
    printf("Opening message queue: %s\n\n", mq_name);  // Debug print to show which queue is being opened
    return 1;  // Return a mock handle for the message queue
}

// Mock for write_mq
int write_mq(mqd_t mq_sender, can_msg *msg) {
    // Print out the message data for debugging purposes
    printf("Writing message to MQ: Identifier: %d, Data: %02X %02X %02X %02X %02X %02X %02X %02X\n\n",
            msg->identifier, msg->dataFrame[0], msg->dataFrame[1], msg->dataFrame[2], 
            msg->dataFrame[3], msg->dataFrame[4], msg->dataFrame[5], msg->dataFrame[6], msg->dataFrame[7]);
    return 0;  // Simulate success in writing to the message queue
}

// Mock for read_mq
int read_mq(mqd_t mq, can_msg *msg) {
    // Simulate a CAN message read from the queue
    /*msg->identifier = ID_SPEED_S;  // Simulate speed sensor data
    msg->dataFrame[0] = 0x00;     // Simulate some speed data (e.g., 100.0 km/h)
    msg->dataFrame[1] = 0x64;
    msg->dataFrame[2] = 0x00;     // Simulate no reverse enabled
    */return 0;  // Simulate success in reading the message
}

// Setup function, called before each test
void setUp(void) {
    // Initial AEB input state setup
    aeb_internal_state.relative_velocity = 0.0;
    aeb_internal_state.has_obstacle = false;        
    aeb_internal_state.obstacle_distance = 0.0;
    aeb_internal_state.brake_pedal = false;
    aeb_internal_state.accelerator_pedal = false;
    aeb_internal_state.on_off_aeb_system = true;
    aeb_internal_state.reverseEnabled = false;
}


// Teardown function, called after each test
void tearDown(void) {
    // No cleanup required for now
}

// Test for the function updateInternalPedalsState
void test_updateInternalPedalsState(void) {
    can_msg captured_frame = { .identifier = ID_PEDALS, .dataFrame = {0x01, 0x00} };

    updateInternalPedalsState(captured_frame);

    TEST_ASSERT_TRUE(aeb_internal_state.accelerator_pedal);  // Accelerator pedal should be ON
    TEST_ASSERT_FALSE(aeb_internal_state.brake_pedal);  // Brake pedal should be OFF

    captured_frame.dataFrame[0] = 0x01;
    captured_frame.dataFrame[1] = 0x01;
    updateInternalPedalsState(captured_frame);

    TEST_ASSERT_TRUE(aeb_internal_state.accelerator_pedal);  // Accelerator pedal should be ON
    TEST_ASSERT_TRUE(aeb_internal_state.brake_pedal);  // Brake pedal should be ON

    captured_frame.dataFrame[0] = 0x00;
    captured_frame.dataFrame[1] = 0x00;
    updateInternalPedalsState(captured_frame);

    TEST_ASSERT_FALSE(aeb_internal_state.accelerator_pedal);  // Accelerator pedal should be OFF
    TEST_ASSERT_FALSE(aeb_internal_state.brake_pedal);  // Brake pedal should be OFF

    captured_frame.dataFrame[0] = 0x00;
    captured_frame.dataFrame[1] = 0x01;
    updateInternalPedalsState(captured_frame);

    TEST_ASSERT_FALSE(aeb_internal_state.accelerator_pedal);  // Accelerator pedal should be OFF
    TEST_ASSERT_TRUE(aeb_internal_state.brake_pedal);  // Brake pedal should be ON
}

// Test for the function updateInternalSpeedState
void test_updateInternalSpeedState(void) {
    can_msg captured_frame = { .identifier = ID_SPEED_S, .dataFrame = {0x00, 0x64, 0x00} };
    
    // Case 1: Normal case where the speed is updated correctly
    updateInternalSpeedState(captured_frame);
    TEST_ASSERT_EQUAL_FLOAT(aeb_internal_state.relative_velocity, 100.0); // 0x64 + (0x00 << 8) = 100.0 * RES_SPEED_S
    
    // Case 2: Case where the CAN data is set to clear data (0xFE, 0xFF)
    captured_frame.dataFrame[0] = 0xFE;
    captured_frame.dataFrame[1] = 0xFF;
    updateInternalSpeedState(captured_frame);
    TEST_ASSERT_EQUAL_FLOAT(aeb_internal_state.relative_velocity, 0.0); // Should reset speed to 0.0
    TEST_ASSERT_FALSE(aeb_internal_state.reverseEnabled); // Should also reset reverseEnabled to false
    
    // Case 3: Case where the CAN data is set to indicate reverse (0x01 in dataFrame[2])
    captured_frame.dataFrame[2] = 0x01;
    updateInternalSpeedState(captured_frame);
    TEST_ASSERT_TRUE(aeb_internal_state.reverseEnabled); // Should enable reverse

    // Case 4: Max speed value constraint (should be capped at 251.0)
    captured_frame.dataFrame[0] = 0xFF; 
    captured_frame.dataFrame[1] = 0xFD; // Maximum possible value
    updateInternalSpeedState(captured_frame);
    TEST_ASSERT_EQUAL_FLOAT(aeb_internal_state.relative_velocity, 251.0); // Should be capped at 251.0
    
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

// Test for the function updateInternalObstacleState
void test_updateInternalObstacleState(void) {
    can_msg captured_frame = { .identifier = ID_OBSTACLE_S, .dataFrame = {0xD0, 0x07, 0x01} };
    
    // Caso 1: Obstáculo detectado e distância calculada corretamente
    updateInternalObstacleState(captured_frame);
    TEST_ASSERT_TRUE(aeb_internal_state.has_obstacle);  // Obstacle detected
    TEST_ASSERT_EQUAL_FLOAT(aeb_internal_state.obstacle_distance, 100.0);  // Expected distance = 100 * RES_OBSTACLE_S
    
    // Caso 2: Nenhum obstáculo (0x00 em dataFrame[2])
    captured_frame.dataFrame[2] = 0x00;
    updateInternalObstacleState(captured_frame);
    TEST_ASSERT_FALSE(aeb_internal_state.has_obstacle);  // No obstacle
    TEST_ASSERT_EQUAL_FLOAT(aeb_internal_state.obstacle_distance, 100.0);  // Distance should remain the same
    // Logic problem: if the obstacle is not detected, the distance should be set to 300.0 (max distance).

    // Caso 3: Limpeza de dados com 0xFE, 0xFF
    captured_frame.dataFrame[0] = 0xFE;
    captured_frame.dataFrame[1] = 0xFF;
    updateInternalObstacleState(captured_frame);
    TEST_ASSERT_FALSE(aeb_internal_state.has_obstacle);  // Obstacle not changed
    TEST_ASSERT_EQUAL_FLOAT(aeb_internal_state.obstacle_distance, 300.0);  // Distance should be set to max value (300.0)
    
    // Caso 4: Dados não precisam de ação (0xFF, 0xFF)
    captured_frame.dataFrame[0] = 0xFF;
    captured_frame.dataFrame[1] = 0xFF;
    updateInternalObstacleState(captured_frame);
    TEST_ASSERT_FALSE(aeb_internal_state.has_obstacle);  // Obstacle not changed
    TEST_ASSERT_EQUAL_FLOAT(aeb_internal_state.obstacle_distance, 0.0);  // Distance should be set to 0.0
    
    // Caso 5: Distância calculada a partir dos dados CAN
    captured_frame.dataFrame[0] = 0xD0;
    captured_frame.dataFrame[1] = 0x07; // Data distance = 0x0201 = 513
    captured_frame.dataFrame[2] = 0x01; // There is an obstacle
    updateInternalObstacleState(captured_frame);
    TEST_ASSERT_TRUE(aeb_internal_state.has_obstacle);  // Obstacle detected
    TEST_ASSERT_EQUAL_FLOAT(aeb_internal_state.obstacle_distance, 100.0);  // Calculated distance
    
    // Caso 6: Distância máxima é limitada a 300.0
    captured_frame.dataFrame[0] = 0xFF;
    captured_frame.dataFrame[1] = 0xFD; // Data distance = 0xFFFF = 65535
    updateInternalObstacleState(captured_frame);
    TEST_ASSERT_TRUE(aeb_internal_state.has_obstacle);  // Obstacle detected
    TEST_ASSERT_EQUAL_FLOAT(aeb_internal_state.obstacle_distance, 300.0);  // Should be capped at 300.0
}


// Test for the function updateInternalCarCState
void test_updateInternalCarCState(void) {
    can_msg captured_frame = { .identifier = ID_CAR_C, .dataFrame = {0x01} };

    // Caso 1: AEB system ON (dataFrame[0] == 0x01)
    updateInternalCarCState(captured_frame);
    TEST_ASSERT_TRUE(aeb_internal_state.on_off_aeb_system);  // AEB system should be ON

    // Caso 2: AEB system OFF (dataFrame[0] == 0x00)
    captured_frame.dataFrame[0] = 0x00;
    updateInternalCarCState(captured_frame);
    TEST_ASSERT_FALSE(aeb_internal_state.on_off_aeb_system);  // AEB system should be OFF

    // Caso 3: Verifica que o sistema AEB permanece OFF para qualquer valor diferente de 0x01
    captured_frame.dataFrame[0] = 0x02;  // Um valor arbitrário, diferente de 0x01
    updateInternalCarCState(captured_frame);
    TEST_ASSERT_FALSE(aeb_internal_state.on_off_aeb_system);  // AEB system should remain OFF

    // Caso 4: Verifica que o sistema AEB permanece ON quando dataFrame[0] for 0x01 novamente
    captured_frame.dataFrame[0] = 0x01;
    updateInternalCarCState(captured_frame);
    TEST_ASSERT_TRUE(aeb_internal_state.on_off_aeb_system);  // AEB system should remain ON
}

// Test for the function getAEBState
void test_getAEBState(void) {
    aeb_internal_state.relative_velocity = 80.0;
    aeb_internal_state.has_obstacle = true;
    aeb_internal_state.obstacle_distance = 10.0;
    aeb_internal_state.brake_pedal = false;
    aeb_internal_state.accelerator_pedal = false;

    double ttc = 1.9;  // Example TTC

    aeb_controller_state state = getAEBState(aeb_internal_state, ttc);
    TEST_ASSERT_EQUAL_INT(state, AEB_STATE_ALARM);  // Should be in ALARM state

    aeb_internal_state.relative_velocity = 60.0;
    ttc = 0.9;
    state = getAEBState(aeb_internal_state, ttc);
    TEST_ASSERT_EQUAL_INT(state, AEB_STATE_BRAKE);  // Should be in BRAKE state if high speed and low TTC
}

// Test for the function translateAndCallCanMsg
void test_translateAndCallCanMsg(void) {
    can_msg captured_frame = { .identifier = ID_PEDALS, .dataFrame = {0x01, 0x00} };
    
    // Test translation and dispatch to pedals update function
    translateAndCallCanMsg(captured_frame);
    TEST_ASSERT_TRUE(aeb_internal_state.accelerator_pedal);  // Accelerator pedal should be ON

    //captured_frame.identifier = ID_EMPTY;
    //TEST_ASSERT_EQUAL_STRING("CAN Identifier unknown", translateAndCallCanMsg(captured_frame));
}

// Main function to run the tests
int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_updateInternalPedalsState);
    RUN_TEST(test_updateInternalSpeedState);
    RUN_TEST(test_updateInternalObstacleState);
    RUN_TEST(test_updateInternalCarCState);
    RUN_TEST(test_getAEBState);
    RUN_TEST(test_translateAndCallCanMsg);
    return UNITY_END();
}