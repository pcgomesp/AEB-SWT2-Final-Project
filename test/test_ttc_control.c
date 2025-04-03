#define UNITY_DOUBLE_SUPPORT
#include "ttc_control.h"
#include <stdio.h>
#include <unistd.h> 
#include <sys/stat.h>
#include "unity.h"
#include <math.h>

double relspeed_test;
double distance_test;
//struct timespec wait_time = {0, 1000000}; // 0.001s

#define delta 0.001 // margin of error

void mywait(){
    struct timespec wait_time = {1, 000000000};
    nanosleep(&wait_time, NULL);
}

void setUp(){
    // set stuff up here
    mywait();
}

void tearDown(){
    // clean stuff up here//
}

// accel_calc
void test_accel_calc_zero_initial() { // Verify return 
    TEST_ASSERT_FLOAT_WITHIN(delta, 0.0, accel_calc(30.0));
}

void test_accel_calc_small_time_frame() { // Verify return 0 when time difference is too small
    accel_calc(30.0);  // First call, to init static
    TEST_ASSERT_FLOAT_WITHIN(delta, 0.0, accel_calc(90.0)); // Expected: ~0 m/s² -> time frame too small
}

// testinggggg

void test_accel_calc_positive_accel() {
    double acel1 = accel_calc(50.0);  // First call, to init static
    printf("%.8lf\n", acel1);
    mywait();
    TEST_ASSERT_FLOAT_WITHIN(delta, 2.777, accel_calc(60.0)); // Aceleração esperada: ~2.777 m/s²
}

void test_accel_calc_positive_accel2() {
    double acel2 = accel_calc(20.0);  // First call, to init static
    printf("%.8lf\n", acel2);
    mywait();
    TEST_ASSERT_FLOAT_WITHIN(delta, 2.777, accel_calc(30.0)); // Aceleração esperada: ~2.777 m/s²
}

void test_accel_calc_negative_accel() {
    accel_calc(60.0);
    mywait();
    TEST_ASSERT_FLOAT_WITHIN(delta, -2.777, accel_calc(50.0)); // Aceleração esperada: ~-2.777 m/s²
}

// ttc_calc//

int main(){
    UNITY_BEGIN();
    //RUN_TEST();
    //RUN_TEST(test_ttc_calc);
    RUN_TEST(test_accel_calc_zero_initial);
    RUN_TEST(test_accel_calc_positive_accel);
    RUN_TEST(test_accel_calc_positive_accel2);
    RUN_TEST(test_accel_calc_small_time_frame);
    RUN_TEST(test_accel_calc_negative_accel);
    // hi
    // changes here
    return UNITY_END();
}