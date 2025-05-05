#include <sys/stat.h>
#include "unity.h"
#include "log_utils.h"
#include "actuators.h"
#include "dbc.h"
#include <time.h>
#include <string.h>

static bool wrap_fopen_fail = false;
static bool wrap_perror_called = false;

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

actuators_abstraction actuators_try;

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
    remove("test/test_log.txt");
}

/**
 * @test
 * @brief Verifies if a fopen error is catchable by a test (using mock functions)
 * 
 * \anchor test_log_event_fopen_fail
 * test ID [TC_LOG_UTILS_001](@ref TC_LOG_UTILS_001)
 */
void test_log_event_fopen_fail(){
    // Try to write:
    // Verify if write
    // If didn't write, assert test
    wrap_fopen_fail = true;
    wrap_perror_called = false;

    // Chama a função sob teste
    log_event("Fopen_Fail_Test", can_frame_test.identifier, actuators_test);

    // Verificações
    TEST_ASSERT_TRUE(wrap_perror_called);
}

/**
 * 
 * @brief Helper function, used to capture the last file of the file
 * @return Type 'actuators_abstraction', according to the data in the file's last line.
 * @note The test may fail if, for any reason, the file cannot be opened.
 *
 */
actuators_abstraction read_line_test(){
    FILE *file = fopen("test/test_log.txt", "r"); // Abrir o arquivo para leitura

    char line[256];

    while (fgets(line, 256, file) != NULL) {  // Ler cada linha do arquivo
        int v1, v2, v3, v4, v5;  // Variáveis para armazenar os números
        if (sscanf(line, "%*[^|] | %d | %d | %d | %d | %d", &v1, &v2, &v3, &v4, &v5) == 5) {
            printf("Valores extraídos: %d %d %d %d %d\n", v1, v2, v3, v4, v5);
        }
        actuators_test.belt_tightness = v1;
        actuators_test.door_lock = v2;
        actuators_test.should_activate_abs = v3;
        actuators_test.alarm_led = v4;
        actuators_test.alarm_buzzer = v5;
    }
    fclose(file);

    return actuators_test;
}

/**
 * @test
 * @brief Verifies if the line written is as expected.
 * 
 * \anchor test_log_event_check_writing_no1
 * test ID [TC_LOG_UTILS_002](@ref TC_LOG_UTILS_002)
 *
 * @note This test depends on the correct operation of file reading and writing functions. 
 * If one of the two fails, this test will also fail.
 *
 */
void test_log_event_check_writing_no1(){
    // Try to write: done
    // Verify if write: done
    // If write correctly, read last line and assert test -> to do
    wrap_fopen_fail = false;

    log_event("Check_Writing", can_frame_test.identifier, actuators_test);
    
    actuators_try = read_line_test();

    TEST_ASSERT_EQUAL(actuators_test.belt_tightness, actuators_try.belt_tightness);
    TEST_ASSERT_EQUAL(actuators_test.door_lock, actuators_try.door_lock);
    TEST_ASSERT_EQUAL(actuators_test.should_activate_abs, actuators_try.should_activate_abs);
    TEST_ASSERT_EQUAL(actuators_test.alarm_led, actuators_try.alarm_led);
    TEST_ASSERT_EQUAL(actuators_test.alarm_buzzer, actuators_try.alarm_buzzer);
}

void test_log_event_file_already_exists(){
    wrap_fopen_fail = false;
    // First call, file created
    log_event("Check_Writing", can_frame_test.identifier, actuators_test);
    // Second call, file created before
    log_event("Check_Writing", can_frame_test.identifier, actuators_test);

    // The same as the previous test 
    actuators_try = read_line_test();
    TEST_ASSERT_EQUAL(actuators_test.belt_tightness, actuators_try.belt_tightness);
    TEST_ASSERT_EQUAL(actuators_test.door_lock, actuators_try.door_lock);
    TEST_ASSERT_EQUAL(actuators_test.should_activate_abs, actuators_try.should_activate_abs);
    TEST_ASSERT_EQUAL(actuators_test.alarm_led, actuators_try.alarm_led);
    TEST_ASSERT_EQUAL(actuators_test.alarm_buzzer, actuators_try.alarm_buzzer);
}

int main(){
    UNITY_BEGIN();
    RUN_TEST(test_log_event_fopen_fail);
    RUN_TEST(test_log_event_check_writing_no1);
    RUN_TEST(test_log_event_file_already_exists);
    return UNITY_END();
}