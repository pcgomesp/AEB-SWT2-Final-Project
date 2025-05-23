#ifndef TTC_CONTROL_H
#define TTC_CONTROL_H

// Necessary libraries
#include <math.h>
#include <time.h>
#include <stdbool.h>

// Function prototypes
double accel_calc(double spd);
double ttc_calc(double dist, double spd, double rel_acel);
void aeb_control(bool *enable_aeb, bool *alarm_cluster, bool *enable_breaking,
                 bool *lk_seatbelt, bool *lk_doors, double *spd, double *dist, double *acel);

#endif