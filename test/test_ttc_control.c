#define UNITY_DOUBLE_SUPPORT
#include "ttc_control.h"
#include <stdio.h>
#include <unistd.h> 
#include <sys/stat.h>
#include "unity.h"
#include <math.h>
#include <stdbool.h>

double relspeed_test;
double distance_test;
//struct timespec wait_time = {0, 1000000}; // 0.001s

#define delta 0.0001 // margin of error

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

void test_ttc_when_delta_zero(){
    double dist[3], vel[3], acel[3], ttc[3];

    dist[0] = 10.0; vel[0] = 18.0; acel[0] = -1.25;
    ttc[0] = ttc_calc(dist[0], vel[0], acel[0]);
    TEST_ASSERT_FLOAT_WITHIN(delta, 4, ttc[0]);

    dist[1] = 60.0; vel[1] = 54.0; acel[1] = -1.875;
    ttc[1] = ttc_calc(dist[1], vel[1], acel[1]);
    TEST_ASSERT_FLOAT_WITHIN(delta, 8, ttc[1]);
}

void test_ttc_when_delta_positive(){
    double dist[3], vel[3], acel[3], ttc[3];

    dist[0] = 20; vel[0] = 59.5; acel[0] = -3;
    ttc[0] = ttc_calc(dist[0], vel[0], acel[0]);
    TEST_ASSERT_FLOAT_WITHIN(delta, 1.3839, ttc[0]);

    dist[1] = 60.0; vel[1] = 56; acel[1] = -2;
    ttc[1] = ttc_calc(dist[1], vel[1], acel[1]);
    TEST_ASSERT_FLOAT_WITHIN(delta, 7.075, ttc[1]);

    dist[2] = 19.41; vel[2] = 45.3116; acel[2] = -1.29;
    ttc[2] = ttc_calc(dist[2], vel[2], acel[2]);
    TEST_ASSERT_FLOAT_WITHIN(delta, 1.6882, ttc[2]);
}

void test_aebcontrol_no_actuators_trigger(){
    bool enable_aeb, alarm_cluster, enable_breaking, lk_seatbelt, lk_doors;
    double spd, dist, acel;

    // Safe situation: TTC high enough so alarm cluster and auto breaking are disabled
    enable_aeb = true;
    dist = 90; spd = 20; acel=0.5;   
    
    aeb_control(&enable_aeb,&alarm_cluster,&enable_breaking,&lk_seatbelt,&lk_doors,
        &spd, &dist, &acel);
    
    TEST_ASSERT_FALSE(alarm_cluster);
    TEST_ASSERT_FALSE(enable_breaking);

    // Unsafe situation, but AEB deactivated by user:
    enable_aeb = false;
    dist = 7.5; spd = 60; acel=2.5;   
    
    aeb_control(&enable_aeb,&alarm_cluster,&enable_breaking,&lk_seatbelt,&lk_doors,
        &spd, &dist, &acel);
    
    TEST_ASSERT_FALSE(alarm_cluster);
    TEST_ASSERT_FALSE(enable_breaking);
}

void test_aeb_worst_situation(){
    bool enable_aeb, alarm_cluster, enable_breaking, lk_seatbelt, lk_doors;
    double spd, dist, acel;

    // Worst situation: TTC low enough that even seatbelts and doors must be
    // accordingly triggered

    enable_aeb = true;
    dist = 5.5; spd = 58.91; acel=2.5;   
    aeb_control(&enable_aeb,&alarm_cluster,&enable_breaking,&lk_seatbelt,&lk_doors,
        &spd, &dist, &acel);
    
    TEST_ASSERT_TRUE(alarm_cluster);
    TEST_ASSERT_TRUE(enable_breaking);
    TEST_ASSERT_TRUE(lk_seatbelt);
    TEST_ASSERT_FALSE(lk_doors);
}

int main(){
    UNITY_BEGIN();
    
    RUN_TEST(test_ttc_when_acel_zero);    
    RUN_TEST(test_ttc_when_delta_negative);    
    RUN_TEST(test_ttc_when_delta_zero);
    RUN_TEST(test_ttc_when_delta_positive);    
    RUN_TEST(test_aebcontrol_no_actuators_trigger);    
    RUN_TEST(test_aeb_worst_situation);    
    
    return UNITY_END();
}