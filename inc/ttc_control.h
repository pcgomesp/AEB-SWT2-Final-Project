#ifndef TTC_CONTROL_H
#define TTC_CONTROL_H

// Necessary libraries
#include <math.h>
#include <time.h>
#include <stdbool.h>

// Function prototypes
float accel_calc(float spd);
float ttc_calc(float dist, float spd);
void aeb_control(bool *enable_aeb, bool *alarm_cluster, bool *enable_breaking, 
                 float *spd, float *dist);

#endif