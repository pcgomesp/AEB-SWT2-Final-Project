#include "unity.h"
#include "file_reader.h"
#include <sys/stat.h>

const char *test_filename = "cts/cenario.txt";
FILE *test_file;

void setUp()
{
    test_file = open_file(test_filename);
}

void tearDown()
{
    if(test_file)
    {
        fclose(test_file);
        test_file = NULL;
    }
}

void test_open_file_not_null_and_skip_header()
{
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

void test_read_sensor_data_valid_data()
{
    sensors_input_data test_sensor_data;

    /* Checks if the function successfully reads
    all the data from second line
    */
    TEST_ASSERT_EQUAL(1, read_sensor_data(test_file, &test_sensor_data));

    /* --- REVIEW --- */
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

void test_read_sensor_data_eof()
{
    sensors_input_data test_sensor_data;

    while (read_sensor_data(test_file, &test_sensor_data) == 1);
  
    TEST_ASSERT_EQUAL(0, read_sensor_data(test_file, &test_sensor_data));

}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_open_file_not_null_and_skip_header);
    RUN_TEST(test_read_sensor_data_valid_data);
    RUN_TEST(test_read_sensor_data_eof);
    return UNITY_END();

}