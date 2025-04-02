#include "unity.h"
#include "actuators.h"
#include "dbc.h"
#include "constants.h"
#include "mq_utils.h"

// Mock para a função open_mq
mqd_t open_mq(char *mq_name) {
    printf("[MOCK] open_mq called with name: %s\n", mq_name);
    return (mqd_t)1; // Retorna um identificador fictício
}

// Mock para a função read_mq
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
    return -1; // Simula fila vazia
}

// Mock para a função log_event
void log_event(const char *id_aeb, uint32_t event_id, actuators_abstraction actuators) {
    printf("[MOCK LOG] ID: %s, Event: 0x%X, BELT: %d, DOOR: %d, ABS: %d, LED: %d, BUZZ: %d\n",
           id_aeb, event_id, actuators.belt_tightness, actuators.door_lock,
           actuators.should_activate_abs, actuators.alarm_led, actuators.alarm_buzzer);
}

// Mock para a variável global actuators_state
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
    // Nada a fazer após cada teste
}

void test_actuatorsTranslateCanMsg_AEB_S_Identifier(void) {
    can_msg test_msg = {
        .identifier = ID_AEB_S,
        .dataFrame = {0xFF, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
    };

    actuatorsTranslateCanMsg(test_msg);

    TEST_ASSERT_TRUE(actuators_state.belt_tightness);
    TEST_ASSERT_FALSE(actuators_state.door_lock);
    TEST_ASSERT_TRUE(actuators_state.should_activate_abs);
    TEST_ASSERT_TRUE(actuators_state.alarm_led);
    TEST_ASSERT_TRUE(actuators_state.alarm_buzzer);
}

// void test_actuatorsTranslateCanMsg_Empty_Identifier(void) {
//     can_msg test_msg = {
//         .identifier = ID_EMPTY,
//         .dataFrame = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
//     };

//     actuatorsTranslateCanMsg(test_msg);

//     TEST_ASSERT_FALSE(actuators_state.belt_tightness);
//     TEST_ASSERT_TRUE(actuators_state.door_lock);
//     TEST_ASSERT_FALSE(actuators_state.should_activate_abs);
//     TEST_ASSERT_FALSE(actuators_state.alarm_led);
//     TEST_ASSERT_FALSE(actuators_state.alarm_buzzer);
// }

// void test_updateInternalActuatorsState_Correct_State(void) {
//     can_msg test_msg = {
//         .identifier = ID_AEB_S,
//         .dataFrame = {0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
//     };

//     updateInternalActuatorsState(test_msg);

//     TEST_ASSERT_FALSE(actuators_state.belt_tightness);
//     TEST_ASSERT_FALSE(actuators_state.door_lock);
//     TEST_ASSERT_FALSE(actuators_state.should_activate_abs);
//     TEST_ASSERT_FALSE(actuators_state.alarm_led);
//     TEST_ASSERT_FALSE(actuators_state.alarm_buzzer);
// }

// Teste simples para a função actuatorsTranslateCanMsg
void test_actuatorsTranslateCanMsg(void) {
    can_msg test_msg = {
        .identifier = ID_AEB_S,
        .dataFrame = {0xFF, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
    };

    actuatorsTranslateCanMsg(test_msg);

    // Verifica se o estado foi atualizado corretamente
    TEST_ASSERT_TRUE(actuators_state.belt_tightness);
    TEST_ASSERT_FALSE(actuators_state.door_lock);
    TEST_ASSERT_TRUE(actuators_state.should_activate_abs);
    TEST_ASSERT_TRUE(actuators_state.alarm_led);
    TEST_ASSERT_TRUE(actuators_state.alarm_buzzer);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_actuatorsTranslateCanMsg_AEB_S_Identifier);
    //RUN_TEST(test_actuatorsTranslateCanMsg_Empty_Identifier);
    //RUN_TEST(test_updateInternalActuatorsState_Correct_State);
    RUN_TEST(test_actuatorsTranslateCanMsg);
    return UNITY_END();
}
