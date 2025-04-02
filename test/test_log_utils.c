#include <sys/stat.h>
#include "unity.h"
#include "log_utils.h"
#include "actuators.h"
#include "dbc.h"
#include <time.h>
#include <string.h>

static bool wrap_fopen_fail = false;
static bool wrap_perror_called = false;
//static const char *test_log_path = "test_log.txt"; // Arquivo temporário

can_msg can_frame_test = {
    .identifier = ID_AEB_S,
    .dataFrame = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}};

can_msg can_frame_empty = {
    .identifier = ID_EMPTY,
    .dataFrame = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}};

actuators_abstraction actuators_test = {
    .belt_tightness = false,
    .door_lock = true,
    .should_activate_abs = false,
    .alarm_led = false,
    .alarm_buzzer = false
};

// FOpen function mock -> Forwards unmocked calls to the real fopen
FILE *__real_fopen(const char *path, const char *mode);
FILE *__wrap_fopen(const char *path, const char *mode) {
    // Intercepts ONLY the path “log/log.txt”
    if (strcmp(path, "log/log.txt") == 0) {
        if (wrap_fopen_fail) {
            return NULL; // Força falha
        } else {
            // Redirects, using real fopen
            return __real_fopen("test/test_log.txt", mode); 
        }
    }
    
    // For other paths, also uses real fopen
    return __real_fopen(path, mode);
}

// Perror mock:  
void __real_perror(const char *s);
void __wrap_perror(const char *s) {
    wrap_perror_called = true;
    __real_perror(s); 
}

void setUp(){
    wrap_fopen_fail = false; // std -> fopen don't fail
    wrap_perror_called = false; // Resets perror state
}

void tearDown(){
    // clean stuff up here
    //remove(test_log_path);
}

// first test: verify if the fopen is catchable by a test (using mock functions)
void test_log_event_fopen_fail(){
    // Try to write:
    // Verify if write
    // If didn't write, assert test
    // Configura o mock para falhar
    wrap_fopen_fail = true;
    wrap_perror_called = false;

    // Chama a função sob teste
    log_event("Fopen_Fail_Test", can_frame_test.identifier, actuators_test);

    // Verificações
    TEST_ASSERT_TRUE(wrap_perror_called);
}

// second test: verify if the last line is written according to the proposed specification
void test_log_event_check_writing(){
    // Try to write: done
    // Verify if write: done
    // If write correctly, read last line and assert test -> to do
    wrap_fopen_fail = false;
    printf("Oie\n");
    log_event("Check_Writing", can_frame_test.identifier, actuators_test);
}


int main(){
    UNITY_BEGIN();
    RUN_TEST(test_log_event_fopen_fail);
    RUN_TEST(test_log_event_check_writing);
    return UNITY_END();
}