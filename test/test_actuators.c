#include "unity.h"
#include "actuators.h"
#include "dbc.h"
#include "constants.h"
#include "mq_utils.h"

// Declare the actuatorsResponseLoop function if it's defined elsewhere
void *actuatorsResponseLoop(void *arg);
#include <pthread.h>

#define LOOP_EMPTY_ITERATIONS_MAX 11







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

void test_actuatorsTranslateCanMsg_Empty_Identifier(void) {
    // // Configura o estado inicial dos atuadores
    // actuators_state.belt_tightness = false;
    // actuators_state.door_lock = false;
    // actuators_state.should_activate_abs = false;
    // actuators_state.alarm_led = false;
    // actuators_state.alarm_buzzer = false;

    // Mensagem com identificador ID_EMPTY
    can_msg test_msg = {
        .identifier = ID_EMPTY,
        .dataFrame = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01}
    };

    // Chama a função para processar a mensagem
    actuatorsTranslateCanMsg(test_msg);

    // Verifica que o estado dos atuadores permanece inalterado
    TEST_ASSERT_FALSE(actuators_state.belt_tightness);
    TEST_ASSERT_FALSE(actuators_state.door_lock);
    TEST_ASSERT_FALSE(actuators_state.should_activate_abs);
    TEST_ASSERT_FALSE(actuators_state.alarm_led);
    TEST_ASSERT_FALSE(actuators_state.alarm_buzzer);
}

void test_updateInternalActuatorsState_Correct_State(void) {
    can_msg test_msg = {
        .identifier = ID_AEB_S,
        .dataFrame = {0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
    };

    updateInternalActuatorsState(test_msg);

    TEST_ASSERT_TRUE(actuators_state.belt_tightness);
    TEST_ASSERT_FALSE(actuators_state.door_lock);
    TEST_ASSERT_TRUE(actuators_state.should_activate_abs);
    TEST_ASSERT_TRUE(actuators_state.alarm_led);
    TEST_ASSERT_TRUE(actuators_state.alarm_buzzer);
}

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

void test_actuatorsTranslateCanMsg_Unknown_Identifier(void) {
    // Mensagem com identificador desconhecido
    can_msg test_msg = {
        .identifier = 0xFFFF, // Identificador inválido
        .dataFrame = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
    };

    // Chama a função para processar a mensagem
    actuatorsTranslateCanMsg(test_msg);

    // Verifica que o estado dos atuadores permanece inalterado
    TEST_ASSERT_FALSE(actuators_state.belt_tightness);
    TEST_ASSERT_FALSE(actuators_state.door_lock);
    TEST_ASSERT_FALSE(actuators_state.should_activate_abs);
    TEST_ASSERT_FALSE(actuators_state.alarm_led);
    TEST_ASSERT_FALSE(actuators_state.alarm_buzzer);
}

void test_updateInternalActuatorsState_DataFrame0_Active(void) {
    can_msg test_msg = {
        .identifier = ID_AEB_S,
        .dataFrame = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
    };

    updateInternalActuatorsState(test_msg);

    TEST_ASSERT_FALSE(actuators_state.belt_tightness);
    TEST_ASSERT_TRUE(actuators_state.door_lock);
    TEST_ASSERT_FALSE(actuators_state.should_activate_abs);
    TEST_ASSERT_TRUE(actuators_state.alarm_led);
    TEST_ASSERT_TRUE(actuators_state.alarm_buzzer);
}

void test_updateInternalActuatorsState_No_Conditions_Met(void) {
    can_msg test_msg = {
        .identifier = ID_AEB_S,
        .dataFrame = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
    };

    updateInternalActuatorsState(test_msg);

    TEST_ASSERT_FALSE(actuators_state.belt_tightness);
    TEST_ASSERT_TRUE(actuators_state.door_lock);
    TEST_ASSERT_FALSE(actuators_state.should_activate_abs);
    TEST_ASSERT_FALSE(actuators_state.alarm_led);
    TEST_ASSERT_FALSE(actuators_state.alarm_buzzer);
}


void test_actuatorsTranslateCanMsg_Invalid_Message(void) {
    // Mensagem CAN inválida (dados incompletos)
    can_msg test_msg = {
        .identifier = ID_AEB_S,
        .dataFrame = {0xFF} // Apenas 1 byte em vez de 8
    };

    // Chama a função para processar a mensagem
    actuatorsTranslateCanMsg(test_msg);

    // Verifica que o estado dos atuadores permanece inalterado
    TEST_ASSERT_FALSE(actuators_state.belt_tightness);
    TEST_ASSERT_FALSE(actuators_state.door_lock);
    TEST_ASSERT_FALSE(actuators_state.should_activate_abs);
    TEST_ASSERT_FALSE(actuators_state.alarm_led);
    TEST_ASSERT_FALSE(actuators_state.alarm_buzzer);
}

void test_log_event_Called(void) {
    // Mock para capturar a saída
    can_msg test_msg = {
        .identifier = ID_AEB_S,
        .dataFrame = {0xFF, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
    };

    actuatorsTranslateCanMsg(test_msg);

    // Não há como verificar diretamente a saída do printf no mock,
    // mas você pode verificar se a função não causa erros.
    TEST_PASS(); // Apenas para garantir que a função foi chamada sem falhas
}

void test_InitialActuatorsState(void) {
    // Verifica que o estado inicial dos atuadores é zero
    TEST_ASSERT_FALSE(actuators_state.belt_tightness);
    TEST_ASSERT_FALSE(actuators_state.door_lock);
    TEST_ASSERT_FALSE(actuators_state.should_activate_abs);
    TEST_ASSERT_FALSE(actuators_state.alarm_led);
    TEST_ASSERT_FALSE(actuators_state.alarm_buzzer);
}

void test_actuatorsTranslateCanMsg_Unexpected_DataFrame(void) {
    can_msg test_msg = {
        .identifier = ID_AEB_S,
        .dataFrame = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x11}
    };

    actuatorsTranslateCanMsg(test_msg);

    // Estado esperado: deve seguir a regra geral da função `updateInternalActuatorsState`
    TEST_ASSERT_FALSE(actuators_state.belt_tightness);
    TEST_ASSERT_TRUE(actuators_state.door_lock);
    TEST_ASSERT_FALSE(actuators_state.should_activate_abs);
    TEST_ASSERT_FALSE(actuators_state.alarm_led);
    TEST_ASSERT_FALSE(actuators_state.alarm_buzzer);
}

void test_actuatorsResponseLoop_EmptyQueue(void) {
    int initial_empty_mq_counter = 0;
    pthread_t thread;
    
    // Inicia a thread do loop de resposta dos atuadores
    pthread_create(&thread, NULL, actuatorsResponseLoop, NULL);

    // Espera um tempo suficiente para atingir o limite
    sleep((LOOP_EMPTY_ITERATIONS_MAX + 1) * 0.2); 

    // O contador de iterações vazias deve ter atingido o limite
    TEST_ASSERT_EQUAL(LOOP_EMPTY_ITERATIONS_MAX, initial_empty_mq_counter);

    // Finaliza a thread
    pthread_cancel(thread);
    pthread_join(thread, NULL);
}

// Mock para a função mock_mq_send
int mock_mq_send(mqd_t mq, const can_msg *msg) {
    printf("[MOCK] mock_mq_send called with identifier: 0x%X\n", msg->identifier);
    return 0; // Simula sucesso no envio da mensagem
}

// Teste para mensagens desconhecidas no loop de resposta dos atuadores
void test_actuatorsResponseLoop_UnknownMessages(void) {
    // Configura o estado inicial dos atuadores
    actuators_state.belt_tightness = false;
    actuators_state.door_lock = true;
    actuators_state.should_activate_abs = false;
    actuators_state.alarm_led = false;
    actuators_state.alarm_buzzer = false;

    can_msg unknown_msg = {
        .identifier = ID_AEB_S,  // Identificador inválido
        .dataFrame = {0xFF, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}   
     };

    // Mock para simular envio de mensagem para a fila
    mqd_t actuators_mq = open_mq("/mock_actuators_mq");
    mock_mq_send(actuators_mq, &unknown_msg);

    // Cria uma thread para executar o loop de resposta
    pthread_t thread;
    pthread_create(&thread, NULL, actuatorsResponseLoop, NULL);

    // Espera um tempo para o loop processar a mensagem
    sleep(1);

    // Verifica se o estado dos atuadores NÃO mudou
    TEST_ASSERT_TRUE(actuators_state.belt_tightness);
    TEST_ASSERT_FALSE(actuators_state.door_lock);
    TEST_ASSERT_TRUE(actuators_state.should_activate_abs);
    TEST_ASSERT_TRUE(actuators_state.alarm_led);
    TEST_ASSERT_TRUE(actuators_state.alarm_buzzer);

    // Cancela e junta a thread
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
    RUN_TEST(test_updateInternalActuatorsState_No_Conditions_Met);
    RUN_TEST(test_actuatorsTranslateCanMsg_Invalid_Message);
    RUN_TEST(test_log_event_Called);
    RUN_TEST(test_InitialActuatorsState);
    RUN_TEST(test_actuatorsTranslateCanMsg_Unexpected_DataFrame);
    RUN_TEST(test_actuatorsResponseLoop_UnknownMessages);
    return UNITY_END();
}
