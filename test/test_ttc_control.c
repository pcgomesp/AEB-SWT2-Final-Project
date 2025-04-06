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
    // struct timespec wait_time = {1, 000000000};
    // nanosleep(&wait_time, NULL);
}

void setUp(){
    // set stuff up here
    // mywait();
}

void tearDown(){
    // clean stuff up here//
}

void test_ttc_when_acel_zero(){
    double dist[3], vel[3], acel[3], ttc[3];

    dist[0] = 100.0; vel[0] = 36.0; acel[0] = 0.0;
    ttc[0] = ttc_calc(dist[0], vel[0], acel[0]);

    dist[1] = 90.0; vel[1] = 55.0; acel[1] = 0.0;
    ttc[1] = ttc_calc(dist[1], vel[1], acel[1]);

    dist[2] = 41.0; vel[2] = 60.0; acel[2] = 4.5;
    ttc[2] = ttc_calc(dist[2], vel[2], acel[2]);

    TEST_ASSERT_FLOAT_WITHIN(delta, 10.0, ttc[0]);
    TEST_ASSERT_FLOAT_WITHIN(delta, 5.8909, ttc[1]);
    TEST_ASSERT_FLOAT_WITHIN(delta, 2.46, ttc[2]);
}

void test_ttc_when_delta_negative(){ 
    // Negative delta -> no real root -> Colision impossible? 
    // Verify this

    double dist[3], vel[3], acel[3], ttc[3];

    dist[0] = 50.0; vel[0] = 15.0; acel[0] = -8.0;
    ttc[0] = ttc_calc(dist[0], vel[0], acel[0]);

    dist[1] = 90.0; vel[1] = 21.0; acel[1] = -4.0;
    ttc[1] = ttc_calc(dist[1], vel[1], acel[1]);

    dist[2] = 41.0; vel[2] = 25.0; acel[2] = -2.87;
    ttc[2] = ttc_calc(dist[2], vel[2], acel[2]);

    TEST_ASSERT_FLOAT_WITHIN(delta, 99, ttc[0]);
    TEST_ASSERT_FLOAT_WITHIN(delta, 99, ttc[1]);
    TEST_ASSERT_FLOAT_WITHIN(delta, 99, ttc[2]);
}

// void test_accel_calc_negative_accel() {
//     accel_calc(60.0);
//     mywait();
//     TEST_ASSERT_FLOAT_WITHIN(delta, -2.777, accel_calc(50.0)); // Aceleração esperada: ~-2.777 m/s²
// }

// ttc_calc//

int main(){
    UNITY_BEGIN();
    //RUN_TEST();

    RUN_TEST(test_ttc_when_acel_zero);    
    RUN_TEST(test_ttc_when_delta_negative);    

    // changes here
    return UNITY_END();
}