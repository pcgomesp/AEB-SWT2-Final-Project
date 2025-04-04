#include "unity.h"
#include <sys/stat.h>
#include "sensors_input.h"
#include "dbc.h"

// Declaration of functions implemented in sensors.c that will be tested
can_msg conv2CANCarClusterData(bool on_off_aeb_system);
can_msg conv2CANVelocityData(bool vehicle_direction, double relative_velocity, double relative_acceleration);
can_msg conv2CANObstacleData(bool has_obstacle, double obstacle_distance);
can_msg conv2CANPedalsData(bool brake_pedal, bool accelerator_pedal);


// Global variables
bool test_on_off_aeb_system;
bool test_vehicle_direction;
bool test_has_obstacle;
bool test_brake_pedal, test_accelerator_pedal;
double test_relative_velocity = 108.0;
double test_obstacle_distance = 60;
double test_relative_acceleration; 


void setUp()
{
    // empty setUp
}

void tearDown()
{
    // empty tearDown
}

void test_conv2CANCarClusterData_AEB_on()
{
    test_on_off_aeb_system = true;
    can_msg result = conv2CANCarClusterData(test_on_off_aeb_system);
    
    TEST_ASSERT_EQUAL_INT(ID_CAR_C, result.identifier);
    TEST_ASSERT_EQUAL_UINT8(0x01, result.dataFrame[0]); // Check if AEB is on

}

void test_conv2CANCarClusterData_AEB_off()
{
    test_on_off_aeb_system = false;
    can_msg result = conv2CANCarClusterData(test_on_off_aeb_system);
    
    TEST_ASSERT_EQUAL_INT(ID_CAR_C, result.identifier);
    TEST_ASSERT_EQUAL_UINT8(0x00, result.dataFrame[0]); // Check if AEB is off
    
}

void test_conv2CANVelocityData_Forward() 
{
    test_vehicle_direction = true;
    test_relative_acceleration = 9.1234;
    can_msg result = conv2CANVelocityData(test_vehicle_direction, test_relative_velocity, test_relative_acceleration);
    
    // vehicle direction data
    TEST_ASSERT_EQUAL_INT(ID_SPEED_S, result.identifier);
    TEST_ASSERT_EQUAL_UINT8(0x01, result.dataFrame[2]); // Check if vehicle direction is forward
    
    // relative velocity data
    unsigned int expected_speed = test_relative_velocity / RES_SPEED_S;
    TEST_ASSERT_EQUAL_UINT8(expected_speed & 0xFF, result.dataFrame[0]); // LS Byte
    TEST_ASSERT_EQUAL_UINT8((expected_speed >> 8) & 0xFF, result.dataFrame[1]); // MS Byte

    // relative acceleration data
    unsigned int expected_accel = ((test_relative_acceleration * RES_ACCELERATION_DIV_S) - OFFSET_ACCELERATION_S);
    TEST_ASSERT_EQUAL_UINT8(expected_accel & 0xFF, result.dataFrame[3]); // LS Byte
    TEST_ASSERT_EQUAL_UINT8((expected_accel >> 8) & 0xFF, result.dataFrame[4]); // MS Byte
    TEST_ASSERT_EQUAL_UINT8(0x00, result.dataFrame[5]); // Acceleration is positive
}

void test_conv2CANVelocityData_Reverse() 
{
    test_vehicle_direction = false;
    test_relative_acceleration = -1.1234;
    can_msg result = conv2CANVelocityData(test_vehicle_direction, test_relative_velocity, test_relative_acceleration);
    
    // vehicle direction data
    TEST_ASSERT_EQUAL_INT(ID_SPEED_S, result.identifier);
    TEST_ASSERT_EQUAL_UINT8(0x00, result.dataFrame[2]); // Check if vehicle direction is reverse
    
    // relative velocity data
    unsigned int expected_speed = test_relative_velocity / RES_SPEED_S;
    TEST_ASSERT_EQUAL_UINT8(expected_speed & 0xFF, result.dataFrame[0]); // least significant byte
    TEST_ASSERT_EQUAL_UINT8((expected_speed >> 8) & 0xFF, result.dataFrame[1]); // most significant bytes

    // relative acceleration data
    double accel_abs = -test_relative_acceleration; // convert to positive
    unsigned int expected_accel = ((accel_abs * RES_ACCELERATION_DIV_S) - OFFSET_ACCELERATION_S);
    TEST_ASSERT_EQUAL_UINT8(expected_accel & 0xFF, result.dataFrame[3]); // LS Byte
    TEST_ASSERT_EQUAL_UINT8((expected_accel >> 8) & 0xFF, result.dataFrame[4]); // MS Byte
    TEST_ASSERT_EQUAL_UINT8(0x01, result.dataFrame[5]); // Acceleration is negative
}

void test_conv2CANObstacleData_Present() 
{
    test_has_obstacle = true;
    can_msg result = conv2CANObstacleData(test_has_obstacle, test_obstacle_distance);
    
    TEST_ASSERT_EQUAL_INT(ID_OBSTACLE_S, result.identifier);
    TEST_ASSERT_EQUAL_UINT8(0x01, result.dataFrame[2]); // Check if obstacle is present
    
    unsigned int expected_distance = test_obstacle_distance / RES_OBSTACLE_S;
    TEST_ASSERT_EQUAL_UINT8(expected_distance & 0xFF, result.dataFrame[0]); // least significant byte
    TEST_ASSERT_EQUAL_UINT8((expected_distance >> 8) & 0xFF, result.dataFrame[1]); // most significant byte
}

void test_conv2CANObstacleData_NotPresent() 
{
    test_has_obstacle = false;
    can_msg result = conv2CANObstacleData(test_has_obstacle, test_obstacle_distance);
    
    TEST_ASSERT_EQUAL_INT(ID_OBSTACLE_S, result.identifier);
    TEST_ASSERT_EQUAL_UINT8(0x00, result.dataFrame[2]); // Check if obstacle is not present
    
    unsigned int expected_distance = test_obstacle_distance / RES_OBSTACLE_S;
    TEST_ASSERT_EQUAL_UINT8(expected_distance & 0xFF, result.dataFrame[0]); // least significant byte
    TEST_ASSERT_EQUAL_UINT8((expected_distance >> 8) & 0xFF, result.dataFrame[1]); // most significant byte
}

void test_conv2CANPedalsData_BrakeAndAccelerator() 
{
    test_brake_pedal = true;
    test_accelerator_pedal = true;
    can_msg result = conv2CANPedalsData(test_brake_pedal, test_accelerator_pedal);
    
    TEST_ASSERT_EQUAL_INT(ID_PEDALS, result.identifier);
    TEST_ASSERT_EQUAL_UINT8(0x01, result.dataFrame[0]); // Accelerator pedal active
    TEST_ASSERT_EQUAL_UINT8(0x01, result.dataFrame[1]); // Brake pedal active
}

void test_conv2CANPedalsData_BrakeOnly() 
{
    test_brake_pedal = true;
    test_accelerator_pedal = false;
    can_msg result = conv2CANPedalsData(test_brake_pedal, test_accelerator_pedal);
    
    TEST_ASSERT_EQUAL_INT(ID_PEDALS, result.identifier);
    TEST_ASSERT_EQUAL_UINT8(0x00, result.dataFrame[0]); // Accelerator pedal inactive
    TEST_ASSERT_EQUAL_UINT8(0x01, result.dataFrame[1]); // Brake pedal active
}

void test_conv2CANPedalsData_AcceleratorOnly() 
{
    test_brake_pedal = false;
    test_accelerator_pedal = true;
    can_msg result = conv2CANPedalsData(test_brake_pedal, test_accelerator_pedal);
    
    TEST_ASSERT_EQUAL_INT(ID_PEDALS, result.identifier);
    TEST_ASSERT_EQUAL_UINT8(0x01, result.dataFrame[0]); // Accelerator pedal active
    TEST_ASSERT_EQUAL_UINT8(0x00, result.dataFrame[1]); // Brake pedal inactive
}

void test_conv2CANPedalsData_NoneActive() 
{
    test_brake_pedal = false;
    test_accelerator_pedal = false;
    can_msg result = conv2CANPedalsData(test_brake_pedal, test_accelerator_pedal);
    
    TEST_ASSERT_EQUAL_INT(ID_PEDALS, result.identifier);
    TEST_ASSERT_EQUAL_UINT8(0x00, result.dataFrame[0]); // Accelerator pedal inactive
    TEST_ASSERT_EQUAL_UINT8(0x00, result.dataFrame[1]); // Brake pedal inactive
}


int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_conv2CANCarClusterData_AEB_on);
    RUN_TEST(test_conv2CANCarClusterData_AEB_off);
    RUN_TEST(test_conv2CANVelocityData_Forward);
    RUN_TEST(test_conv2CANVelocityData_Reverse);
    RUN_TEST(test_conv2CANObstacleData_Present);
    RUN_TEST(test_conv2CANObstacleData_NotPresent);
    RUN_TEST(test_conv2CANPedalsData_BrakeAndAccelerator);
    RUN_TEST(test_conv2CANPedalsData_BrakeOnly);
    RUN_TEST(test_conv2CANPedalsData_AcceleratorOnly);
    RUN_TEST(test_conv2CANPedalsData_NoneActive);
    return UNITY_END();

}