#define UNITY_DOUBLE_SUPPORT
#include "ttc_control.h"
#include <stdio.h>
#include <unistd.h> 
#include <sys/stat.h>
#include "unity.h"
#include <math.h>

double relspeed_test;
double distance_test;

#define delta 0.001 // margin of error

void setUp(){
    // set stuff up here
}

void tearDown(){
    // clean stuff up here//
}

// void test_ttc_calc(){ 
//     // 1 problem found below: it is always stopping in "if (a == 0) return ttc = c / b;"
//     // distance_test = 100;
//     // relspeed_test = 36;
//     // TEST_ASSERT_EQUAL(10, ttc_calc(distance_test, relspeed_test));

//     // distance_test = 100;
//     // relspeed_test = 72;
//     // TEST_ASSERT_EQUAL(5, ttc_calc(distance_test, relspeed_test));

//     // distance_test = 150;
//     // relspeed_test = 54;
//     // TEST_ASSERT_EQUAL(10, ttc_calc(distance_test, relspeed_test));

//     // distance_test = 180;
//     // relspeed_test = 64.8;
//     // TEST_ASSERT_EQUAL(10, ttc_calc(distance_test, relspeed_test));

//     double value1, value2;
//     distance_test = 100;
//     relspeed_test = 30;

//     value1 = ttc_calc(distance_test, relspeed_test);
//     printf("%lf\n",value1);
//     TEST_ASSERT_DOUBLE_WITHIN(0.0001, value1, 12.0);

//     // usleep(1500000);

//     // distance_test = 200;
//     // relspeed_test = 72;

//     // value2 =  ttc_calc(distance_test, relspeed_test);
    
//     // TEST_ASSERT_EQUAL(10, value2);
// }

// accel_calc
void test_accel_calc_zero_initial() { // Verify return 0
    TEST_ASSERT_FLOAT_WITHIN(delta, 0.0, accel_calc(60.0));
}

void test_accel_calc_small_time_frame() { // Verify return 0 when time difference is too small
    accel_calc(50.0);  // First call, to init static
    //struct timespec wait_time = {0, 1000000}; // 0.001s
    //nanosleep(&wait_time, NULL);
    TEST_ASSERT_FLOAT_WITHIN(delta, 0.0, accel_calc(60.0)); // Expected: ~0 m/s² -> time frame too small
    struct timespec wait_time = {2, 000000000}; // Espera 2s
    nanosleep(&wait_time, NULL);
}
//

void test_accel_calc_positive_accel() {
    double acel1 = accel_calc(50.0);  // First call, to init static
    printf("%.8lf\n", acel1);
    struct timespec wait_time = {2, 000000000}; // Espera 2s
    nanosleep(&wait_time, NULL);
    TEST_ASSERT_FLOAT_WITHIN(delta, 2.777, accel_calc(60.0)); // Aceleração esperada: ~2.777 m/s²
}

void test_accel_calc_positive_accel2() {
    double acel2 = accel_calc(50.0);  // First call, to init static
    printf("%.8lf\n", acel2);
    struct timespec wait_time = {2, 000000000}; // Espera 2s
    nanosleep(&wait_time, NULL);
    TEST_ASSERT_FLOAT_WITHIN(delta, 2.777, accel_calc(60.0)); // Aceleração esperada: ~2.777 m/s²
}

void test_accel_calc_negative_accel() {
    accel_calc(60.0);
    struct timespec wait_time = {2, 000000000}; // Espera 0.5s
    nanosleep(&wait_time, NULL);
    TEST_ASSERT_FLOAT_WITHIN(delta, -2.777, accel_calc(50.0)); // Aceleração esperada: ~-2.777 m/s²
}

// ttc_calc//

int main(){
    UNITY_BEGIN();
    //RUN_TEST();
    //RUN_TEST(test_ttc_calc);
    RUN_TEST(test_accel_calc_zero_initial);
    RUN_TEST(test_accel_calc_small_time_frame);
    RUN_TEST(test_accel_calc_positive_accel);
    RUN_TEST(test_accel_calc_positive_accel2);
    RUN_TEST(test_accel_calc_negative_accel);
    return UNITY_END();
}