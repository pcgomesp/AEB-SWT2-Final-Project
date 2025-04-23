#include "unity.h"
#include "file_reader.h"
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <setjmp.h>


static bool wrap_fopen_fail = false;
static bool wrap_perror_called = false;
static bool wrap_exit_called = false;
static int wrap_exit_status = 0;

FILE *test_file;
jmp_buf exit_env;

// ---- WRAPS ------

/** @brief fopen function mock */
FILE *__real_fopen(const char *path, const char *mode);
FILE *__wrap_fopen(const char *path, const char *mode) 
{
    if(wrap_fopen_fail)
        return NULL;
    return __real_fopen(path, mode);
}

/** @brief perror function mock */
void __real_perror(const char *s);
void __wrap_perror(const char *s) 
{
    wrap_perror_called = true;
    __real_perror(s);
}

/** @brief exit function mock */
void __wrap_exit(int status) 
{
    wrap_exit_called = true;
    wrap_exit_status = status;
    longjmp(exit_env, 1);
}

/**
 * @brief setUp function
 */
void setUp()
{
    wrap_fopen_fail = false;
    wrap_perror_called = false;
    wrap_exit_called = false;
    wrap_exit_status = 0; 
}

/**
 * @brief tearDown function
 */
void tearDown()
{
    if (test_file) 
    {
        fclose(test_file);
        test_file = NULL;
    }
    
}

/** 
 * @test
 * @brief Test open_file() when fopen fails
 * \anchor test_open_file_fopen_fail_should_exit
 * test ID [TC_FILE_READER_001](@ref TC_FILE_READER_001)
 */
void test_open_file_fopen_fail_should_exit() 
{
    wrap_fopen_fail = true;
   
    // Test Case ID: TC_FILE_READER_001
    
    if (setjmp(exit_env) == 0) {
        open_file("invalid/path.txt");  // Vai cair no longjmp da wrap_exit
        TEST_FAIL_MESSAGE("exit() was not called as expected");
    }

    TEST_ASSERT_TRUE(wrap_perror_called);
    TEST_ASSERT_TRUE(wrap_exit_called);
    TEST_ASSERT_EQUAL(EXIT_FAILURE, wrap_exit_status);
}

/** 
 * @test
 * @brief Test for the function open_file on file_reader.c [SwR-9] (@ref SwR-9), [SwR-11] (@ref SwR-11)
 * \anchor test_open_file_not_null_and_skip_header
 * test ID [TC_FILE_READER_002](@ref TC_FILE_READER_002)
*/
void test_open_file_not_null_and_skip_header()
{
    const char *test_filename = "tcs/cenario.txt";
    test_file = open_file(test_filename);

    // Test Case ID: TC_FILE_READER_002

    // Checks if test_file pointer is not null
    TEST_ASSERT_NOT_NULL(test_file);

    char buffer[100];
    fgets(buffer, sizeof(buffer), test_file);
    
    /* if buffer is the same as the second line of the file, 
    it means test_file points to the correct line, skipping
    the header
    */
    TEST_ASSERT_EQUAL_STRING("60 1 108 0 1 1 0 0\n", buffer);
   
}

/** 
 * @test
 * @brief Tests for the function read_sensor_data on file_reader.c [SwR-9] (@ref SwR-9), [SwR-11] (@ref SwR-11)
 * \anchor test_read_sensor_data_valid_data
 * test ID [TC_FILE_READER_003](@ref TC_FILE_READER_003)
*/
void test_read_sensor_data_valid_data()
{
    sensors_input_data test_sensor_data;
    const char *test_filename = "tcs/cenario.txt";
    test_file = open_file(test_filename);

    // Test Case ID: TC_FILE_READER_003
    /* Checks if the function successfully reads
    all the data from second line
    */
    TEST_ASSERT_EQUAL(1, read_sensor_data(test_file, &test_sensor_data));

    // Checks if values match expected ones from second line
    TEST_ASSERT_EQUAL_FLOAT(60.0, test_sensor_data.obstacle_distance);
    TEST_ASSERT_EQUAL(1, test_sensor_data.has_obstacle);
    TEST_ASSERT_EQUAL_FLOAT(108.0, test_sensor_data.relative_velocity);
    TEST_ASSERT_EQUAL(0, test_sensor_data.brake_pedal);
    TEST_ASSERT_EQUAL(1, test_sensor_data.accelerator_pedal);
    TEST_ASSERT_EQUAL(1, test_sensor_data.on_off_aeb_system);
    TEST_ASSERT_EQUAL(0, test_sensor_data.reverseEnabled);
    TEST_ASSERT_EQUAL_FLOAT(0.0,test_sensor_data.relative_acceleration);

}

/**  @test 
 * \anchor test_read_sensor_data_eof
 * test ID [TC_FILE_READER_004](@ref TC_FILE_READER_004)
*/
void test_read_sensor_data_eof()
{
    sensors_input_data test_sensor_data;
    const char *test_filename = "tcs/cenario.txt";
    test_file = open_file(test_filename);

    // Test Case ID: TC_FILE_READER_004
    while (read_sensor_data(test_file, &test_sensor_data) == 1);
  
    TEST_ASSERT_EQUAL(0, read_sensor_data(test_file, &test_sensor_data));
}




int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_open_file_fopen_fail_should_exit);
    RUN_TEST(test_open_file_not_null_and_skip_header);
    RUN_TEST(test_read_sensor_data_valid_data);
    RUN_TEST(test_read_sensor_data_eof);
    return UNITY_END();

}