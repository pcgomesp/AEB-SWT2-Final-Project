#include "unity.h"
#include "actuators.h"

// Define a estrutura can_msg caso ainda não esteja definida
// Esta estrutura representa uma mensagem CAN com um identificador e um quadro de dados de 8 bytes
typedef struct {
    int identifier; // Identificador da mensagem CAN
    unsigned char dataFrame[8]; // Quadro de dados da mensagem CAN (8 bytes)
} can_msg;

// Define the missing identifiers
#define ID_AEB_S 0x100
#define ID_EMPTY 0x000

// Mocks para CAN messages
can_msg test_msg_1 = { .identifier = ID_AEB_S, .dataFrame = {0x00, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF} };
can_msg test_msg_2 = { .identifier = ID_AEB_S, .dataFrame = {0x01, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF} };
can_msg test_msg_empty = { .identifier = ID_EMPTY, .dataFrame = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} };

// Define the actuators_state structure
typedef struct {
    bool belt_tightness;
    bool door_lock;
    bool should_activate_abs;
    bool alarm_led;
    bool alarm_buzzer;
} ActuatorsState;

// Declare a global variable for actuators_state
ActuatorsState actuators_state;

// Setup para rodar antes de cada teste
void setUp(void) {
    actuators_state.belt_tightness = false;
    actuators_state.door_lock = true;
    actuators_state.should_activate_abs = false;
    actuators_state.alarm_led = false;
    actuators_state.alarm_buzzer = true;
}

// Cleanup após cada teste
void tearDown(void) {}

// Teste 1: Verifica se os atuadores mudam corretamente ao receber uma mensagem CAN válida
void test_updateInternalActuatorsState_case1(void) {
    updateInternalActuatorsState(test_msg_1);
    TEST_ASSERT_TRUE(actuators_state.belt_tightness);
    TEST_ASSERT_FALSE(actuators_state.door_lock);
    TEST_ASSERT_TRUE(actuators_state.should_activate_abs);
    TEST_ASSERT_TRUE(actuators_state.alarm_led);
    TEST_ASSERT_TRUE(actuators_state.alarm_buzzer);
}

// Teste 2: Outra condição de entrada
void test_updateInternalActuatorsState_case2(void) {
    updateInternalActuatorsState(test_msg_2);
    TEST_ASSERT_FALSE(actuators_state.belt_tightness);
    TEST_ASSERT_TRUE(actuators_state.door_lock);
    TEST_ASSERT_FALSE(actuators_state.should_activate_abs);
    TEST_ASSERT_TRUE(actuators_state.alarm_led);
    TEST_ASSERT_TRUE(actuators_state.alarm_buzzer);
}

// Teste 3: Mensagem vazia não deve ativar nada
void test_actuatorsTranslateCanMsg_empty(void) {
    actuatorsTranslateCanMsg(test_msg_empty);
    TEST_ASSERT_FALSE(actuators_state.belt_tightness);
    TEST_ASSERT_TRUE(actuators_state.door_lock);
    TEST_ASSERT_FALSE(actuators_state.should_activate_abs);
    TEST_ASSERT_FALSE(actuators_state.alarm_led);
    TEST_ASSERT_FALSE(actuators_state.alarm_buzzer);
}

// Função principal dos testes
int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_updateInternalActuatorsState_case1);
    RUN_TEST(test_updateInternalActuatorsState_case2);
    RUN_TEST(test_actuatorsTranslateCanMsg_empty);
    return UNITY_END();
}
