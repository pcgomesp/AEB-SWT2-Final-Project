#include "unity.h"
#include "actuators.h"
#include "dbc.h"
#include "constants.h"
#include "mq_utils.h"
#include <unistd.h>

// Declare the actuatorsResponseLoop function if it's defined elsewhere
void *actuatorsResponseLoop(void *arg);
#include <pthread.h>

#define LOOP_EMPTY_ITERATIONS_MAX 11


// Mock to open_mq function
mqd_t open_mq(char *mq_name) {
    printf("[MOCK] open_mq called with name: %s\n", mq_name);
    return (mqd_t)1; // Return a mock mqd_t value
}

// Mock to read_mq
int read_mq(mqd_t mq, can_msg *msg) {
    static int counter = 0;
    if (counter == 0) {
        msg->identifier = ID_AEB_S;
        msg->dataFrame[0] = 0x01;
        msg->dataFrame[1] = 0x01;
        counter++;
        return 0;
    } else if (counter == 1) {
        msg->identifier = ID_EMPTY;
        counter++;
        return 0;
    }
    return -1; // EMpty queue
}



// Mock to log_event
void log_event(const char *id_aeb, uint32_t event_id, actuators_abstraction actuators) {
    printf("[MOCK LOG] ID: %s, Event: 0x%X, BELT: %d, DOOR: %d, ABS: %d, LED: %d, BUZZ: %d\n",
           id_aeb, event_id, actuators.belt_tightness, actuators.door_lock,
           actuators.should_activate_abs, actuators.alarm_led, actuators.alarm_buzzer);
}

// Mock to global variable actuators_state
extern actuators_abstraction actuators_state;

// Funções de setup e teardown
void setUp(void) {
    actuators_state.belt_tightness = 0;
    actuators_state.door_lock = 0;
    actuators_state.should_activate_abs = 0;
    actuators_state.alarm_led = 0;
    actuators_state.alarm_buzzer = 0;
}

void tearDown(void) {
    // Nothing to do here
    // This function is called after each test
}

/**
 * @test
 * @brief Verifies if actuatorsTranslateCanMsg correctly processes a message with ID_AEB_S
 * 
 * \anchor test_actuatorsTranslateCanMsg_AEB_S_Identifier
 * test ID [TC_AEB_A__001](@ref TC_AEB_A__001)
 */
void test_actuatorsTranslateCanMsg_AEB_S_Identifier(void) {
    can_msg test_msg = {
        .identifier = ID_AEB_S,
        .dataFrame = {0xFF, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
    };

    actuatorsTranslateCanMsg(test_msg);

    // Checks if the actuators' state was updated correctly
    //// Test case ID: TC_AEB_A__001
    TEST_ASSERT_TRUE(actuators_state.belt_tightness);
    TEST_ASSERT_FALSE(actuators_state.door_lock);
    TEST_ASSERT_TRUE(actuators_state.should_activate_abs);
    TEST_ASSERT_TRUE(actuators_state.alarm_led);
    TEST_ASSERT_TRUE(actuators_state.alarm_buzzer);
}

/**
 * @test
 * @brief Verifies if actuatorsTranslateCanMsg handles a message with ID_EMPTY correctly
 * \anchor test_actuatorsTranslateCanMsg_Empty_Identifier
 * test ID [TC_AEB_A__002](@ref TC_AEB_A__002)
 */
void test_actuatorsTranslateCanMsg_Empty_Identifier(void) {
    can_msg test_msg = {
        .identifier = ID_EMPTY,
        .dataFrame = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01}
    };

    actuatorsTranslateCanMsg(test_msg);

    // Checks that the actuators' state remains unchanged
    //// Test case ID: TC_AEB_A__002
    TEST_ASSERT_FALSE(actuators_state.belt_tightness);
    TEST_ASSERT_FALSE(actuators_state.door_lock);
    TEST_ASSERT_FALSE(actuators_state.should_activate_abs);
    TEST_ASSERT_FALSE(actuators_state.alarm_led);
    TEST_ASSERT_FALSE(actuators_state.alarm_buzzer);
}

/**
 * @test
 * @brief Verifies if actuatorsTranslateCanMsg handles a message with ID_EMPTY correctly
 * \anchor test_updateInternalActuatorsState_Correct_State
 * test ID [TC_AEB_A__003](@ref TC_AEB_A__003)
 */
void test_updateInternalActuatorsState_Correct_State(void) {
    // Verifies if updateInternalActuatorsState correctly updates the actuators' state
    can_msg test_msg = {
        .identifier = ID_AEB_S,
        .dataFrame = {0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
    };

    updateInternalActuatorsState(test_msg);

    // Checks the expected state of the actuators
    //// Test case ID: TC_AEB_A__003
    TEST_ASSERT_TRUE(actuators_state.belt_tightness);
    TEST_ASSERT_FALSE(actuators_state.door_lock);
    TEST_ASSERT_TRUE(actuators_state.should_activate_abs);
    TEST_ASSERT_TRUE(actuators_state.alarm_led);
    TEST_ASSERT_TRUE(actuators_state.alarm_buzzer);
}

/**
 * @test
 * @brief Verifies if actuatorsTranslateCanMsg correctly processes a valid message.
* \anchor test_actuatorsTranslateCanMsg
 * test ID [TC_AEB_A__004](@ref TC_AEB_A__004)
 */
void test_actuatorsTranslateCanMsg(void) {
    can_msg test_msg = {
        .identifier = ID_AEB_S,
        .dataFrame = {0xFF, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
    };

    actuatorsTranslateCanMsg(test_msg);

    // Checks if the state was updated correctly
        //// Test case ID: TC_AEB_A__004
    TEST_ASSERT_TRUE(actuators_state.belt_tightness);
    TEST_ASSERT_FALSE(actuators_state.door_lock);
    TEST_ASSERT_TRUE(actuators_state.should_activate_abs);
    TEST_ASSERT_TRUE(actuators_state.alarm_led);
    TEST_ASSERT_TRUE(actuators_state.alarm_buzzer);
}

/**
 * @test
 * @brief Verifies if actuatorsTranslateCanMsg correctly processes a valid message.
 * \anchor test_actuatorsTranslateCanMsg_Unknown_Identifier
 * test ID [TC_AEB_A__005](@ref TC_AEB_A__005)
 */
void test_actuatorsTranslateCanMsg_Unknown_Identifier(void) {
    // Verifies if actuatorsTranslateCanMsg handles an unknown identifier correctly
    can_msg test_msg = {
        .identifier = 0xFFFF, // Invalid identifier
        .dataFrame = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
    };

    actuatorsTranslateCanMsg(test_msg);

    // Checks that the actuators' state remains unchanged
    //// Test case ID: TC_AEB_A__005
    TEST_ASSERT_FALSE(actuators_state.belt_tightness);
    TEST_ASSERT_FALSE(actuators_state.door_lock);
    TEST_ASSERT_FALSE(actuators_state.should_activate_abs);
    TEST_ASSERT_FALSE(actuators_state.alarm_led);
    TEST_ASSERT_FALSE(actuators_state.alarm_buzzer);
}

/** @test 
 * @brief Verifies if updateInternalActuatorsState processes correctly when dataFrame[0] == 0x01
 * \anchor test_updateInternalActuatorsState_DataFrame0_Active
 * test ID [TC_AEB_A__006](@ref TC_AEB_A__006)
*/
void test_updateInternalActuatorsState_DataFrame0_Active(void) {
    // Verifies if updateInternalActuatorsState processes correctly when dataFrame[0] == 0x01
    can_msg test_msg = {
        .identifier = ID_AEB_S,
        .dataFrame = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
    };

    updateInternalActuatorsState(test_msg);

    // Checks the expected state of the actuators
    //// Test case ID: TC_AEB_A__006
    TEST_ASSERT_FALSE(actuators_state.belt_tightness);
    TEST_ASSERT_TRUE(actuators_state.door_lock);
    TEST_ASSERT_FALSE(actuators_state.should_activate_abs);
    TEST_ASSERT_TRUE(actuators_state.alarm_led);
    TEST_ASSERT_TRUE(actuators_state.alarm_buzzer);
}





/** @test
 * @brief Verifies the initial state of the actuators
 * \anchor test_InitialActuatorsState
 * test ID [TC_AEB_A__007](@ref TC_AEB_A__007)
 */
void test_InitialActuatorsState(void) {
    // Verifies that the initial state of the actuators is zero
    //// Test case ID: TC_AEB_A__007
    TEST_ASSERT_FALSE(actuators_state.belt_tightness);
    TEST_ASSERT_FALSE(actuators_state.door_lock);
    TEST_ASSERT_FALSE(actuators_state.should_activate_abs);
    TEST_ASSERT_FALSE(actuators_state.alarm_led);
    TEST_ASSERT_FALSE(actuators_state.alarm_buzzer);
}

/** @test 
 * @brief Verifies if actuatorsTranslateCanMsg handles an unexpected dataFrame correctly
 * \anchor test_actuatorsTranslateCanMsg_Unexpected_DataFrame
 * test ID [TC_AEB_A__008](@ref TC_AEB_A__008)
*/
void test_actuatorsTranslateCanMsg_Unexpected_DataFrame(void) {
    // Verifies if actuatorsTranslateCanMsg handles an unexpected dataFrame correctly
    can_msg test_msg = {
        .identifier = ID_AEB_S,
        .dataFrame = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x11}
    };

    actuatorsTranslateCanMsg(test_msg);

    // Expected state: should follow the general rule of the `updateInternalActuatorsState` function
    //// Test case ID: TC_AEB_A__008
    TEST_ASSERT_FALSE(actuators_state.belt_tightness);
    TEST_ASSERT_TRUE(actuators_state.door_lock);
    TEST_ASSERT_FALSE(actuators_state.should_activate_abs);
    TEST_ASSERT_FALSE(actuators_state.alarm_led);
    TEST_ASSERT_FALSE(actuators_state.alarm_buzzer);
}

/**
 * @test
 * @brief Verifies if the actuators response loop handles an empty queue correctly
 * \anchor test_actuatorsResponseLoop_EmptyQueue
 * test ID [TC_AEB_A__009](@ref TC_AEB_A__009) */
void test_actuatorsResponseLoop_EmptyQueue(void) {
    //// Test case ID: TC_AEB_A__009

    int initial_empty_mq_counter = 0;
    pthread_t thread;

    // Starts the actuators response loop thread
    pthread_create(&thread, NULL, actuatorsResponseLoop, NULL);

    // Waits enough time to reach the limit
    sleep((LOOP_EMPTY_ITERATIONS_MAX + 1) * 0.2);

    // Checks if the empty iterations counter reached the limit

    TEST_ASSERT_EQUAL(LOOP_EMPTY_ITERATIONS_MAX, initial_empty_mq_counter);

    // Cancels and joins the thread
    pthread_cancel(thread);
    pthread_join(thread, NULL);
}

// Mock for mock_mq_send
int mock_mq_send(mqd_t mq, const can_msg *msg) {
    printf("[MOCK] mock_mq_send called with identifier: 0x%X\n", msg->identifier);
    return 0; // Simula sucesso no envio da mensagem
}

/**
 * @test
 * @brief Verifies if the actuators response loop handles unknown messages correctly
 * \anchor test_actuatorsResponseLoop_UnknownMessages
 * test ID [TC_AEB_A__010](@ref TC_AEB_A__010)
 */
void test_actuatorsResponseLoop_UnknownMessages(void) {
    //// Test case ID: TC_AEB_A__010
    actuators_state.belt_tightness = false;
    actuators_state.door_lock = true;
    actuators_state.should_activate_abs = false;
    actuators_state.alarm_led = false;
    actuators_state.alarm_buzzer = false;

    can_msg unknown_msg = {
        .identifier = ID_AEB_S,  // Invalid identifier
        .dataFrame = {0xFF, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
    };

    // Mock to simulate sending a message to the queue
    mqd_t actuators_mq = open_mq("/mock_actuators_mq");
    mock_mq_send(actuators_mq, &unknown_msg);

    // Creates a thread to execute the response loop
    pthread_t thread;
    pthread_create(&thread, NULL, actuatorsResponseLoop, NULL);

    // Waits for the loop to process the message
    sleep(1);

    // Checks that the actuators' state did NOT change
    //// Test case ID: TC_AEB_A__011
    TEST_ASSERT_TRUE(actuators_state.belt_tightness);
    TEST_ASSERT_FALSE(actuators_state.door_lock);
    TEST_ASSERT_TRUE(actuators_state.should_activate_abs);
    TEST_ASSERT_TRUE(actuators_state.alarm_led);
    TEST_ASSERT_TRUE(actuators_state.alarm_buzzer);

    // Cancels and joins the thread
    pthread_cancel(thread);
    pthread_join(thread, NULL);
}


int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_actuatorsTranslateCanMsg_AEB_S_Identifier);
    RUN_TEST(test_actuatorsTranslateCanMsg_Empty_Identifier);
    RUN_TEST(test_updateInternalActuatorsState_Correct_State);
    RUN_TEST(test_actuatorsTranslateCanMsg);
    RUN_TEST(test_actuatorsTranslateCanMsg_Unknown_Identifier);
    RUN_TEST(test_updateInternalActuatorsState_DataFrame0_Active);
    RUN_TEST(test_InitialActuatorsState);
    RUN_TEST(test_actuatorsTranslateCanMsg_Unexpected_DataFrame);
    RUN_TEST(test_actuatorsResponseLoop_UnknownMessages);
    return UNITY_END();
}
