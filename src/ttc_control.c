#include "ttc_control.h"
#include "constants.h"

/**
 * @brief Calculate acceleration based on speed and time difference.
 *
 * This function calculates the acceleration in m/s² based on the difference in speed and time. 
 * It uses the `clock_gettime` function to measure the time difference between consecutive calls 
 * and calculates acceleration as the change in speed divided by the elapsed time [SwR-11].
 * 
 * @param spd Current speed in km/h.
 * 
 * @return The acceleration in m/s². If the function is called for the first time or there is
 *         insufficient time difference between calls, it returns 0.0.
 */
double accel_calc(double spd) {
    static double prev_spd = 0.0;
    static struct timespec start_time = {0, 0};
    struct timespec current_time;
    double elapsed_time;
    double accel = 0.0;

    clock_gettime(CLOCK_REALTIME, &current_time);

    if (prev_spd == 0.0) {
        prev_spd = spd;
        start_time = current_time;
        return 0.0;
    }

    else {
        elapsed_time = (double)(current_time.tv_sec - start_time.tv_sec) 
                        + (double)(current_time.tv_nsec - start_time.tv_nsec) / 1e9;
        if (elapsed_time < 0.01) return 0.0;
        
        accel = ((spd - prev_spd) / 3.6) / elapsed_time;
        start_time = current_time;
        prev_spd = spd;
        
        return accel;
    } 
}

/**
 * @brief Calculate the time to collision (TTC) based on relative distance and speed.
 *
 * This function calculates the time to collision using the Uniformly Variable Motion (UVM) model. 
 * It uses a quadratic equation to calculate the time to collision (TTC) between two objects 
 * based on their relative distance and speed. If the relative acceleration is zero, the function 
 * simply calculates the time by dividing the relative distance by the relative speed [SwR-1], [SwR-10], [SwR-11]. 
 *
 * @param dis_rel The relative distance between the objects in meters.
 * @param spd_rel The relative speed between the objects in km/h.
 * 
 * @return The time to collision in seconds. If no real solution is found (i.e., negative discriminant),
 *         the function returns -1.0. If there is no relative acceleration, the time to collision is calculated
 *         as distance divided by speed.
 */
double ttc_calc(double dis_rel, double spd_rel) {
    double a, b, c, ttc, delta;
    
    a = accel_calc(spd_rel);
    b = spd_rel / 3.6;
    c = dis_rel;

    if (a == 0) return ttc = c / b;

    delta = b * b + 2 * a * c;
    
    if (delta < 0) return -1.0;

    else if (delta == 0) return -b / a;

    else {
        ttc = (-b + sqrt(delta)) / a;
        return ttc;
    }
}

/**
 * @brief Controls the Autonomous Emergency Braking (AEB) system.
 * 
 * This function calculates the Time to Collision (TTC) based on the relative distance
 * and speed of the vehicle. If the TTC is below a certain threshold, the AEB system
 * triggers the alarm and may engage the braking system. It also prepares other safety 
 * features like seatbelt locking and door unlocking in critical conditions.
 * 
 * The decision is made based on predefined TTC thresholds:
 * - If the TTC is below the `THRESHOLD_ALARM` (e.g., 2 seconds), an alarm is triggered.
 * - If the TTC is below the `THRESHOLD_BRAKING` (e.g., 1 second), the braking system is activated.
 * - If the TTC is less than half of the braking threshold, the braking system is prepared for a collision.
 * 
 * The AEB system and the alarm are only triggered if the `enable_aeb` flag is set to true.
 * 
 * @param enable_aeb A pointer to a boolean flag that enables or disables the AEB system. [SwR-5]
 * @param alarm_cluster A pointer to a boolean flag that triggers the alarm in the cluster. [SwR-2]
 * @param enable_breaking A pointer to a boolean flag that enables or disables the braking system. [SwR-3]
 * @param lk_seatbelt A pointer to a boolean flag that locks the seatbelt in case of emergency. [Sys-F-14]
 * @param lk_doors A pointer to a boolean flag that locks or unlocks the doors in an emergency. [Sys-F-14]
 * @param spd A pointer to the current speed of the vehicle (in km/h).
 * @param dist A pointer to the current distance to the obstacle (in meters).
 * 
 * @note The function complies with the requirements (SwR-2, SwR-3, SwR-5,
 *       SwR-11, SwR-14, SwR-15, Sys-F-8, Sys-F-9, Sys-F-14).
 */
void aeb_control(bool *enable_aeb, bool *alarm_cluster, bool *enable_breaking,
                 bool *lk_seatbelt, bool *lk_doors, double *spd, double *dist) {
    double ttc;
    
    ttc = ttc_calc(*dist, *spd);
    
    if ((*enable_aeb) && (ttc > 0.0) && (ttc < THRESHOLD_ALARM)) {
        *alarm_cluster = true;
        
        if ((ttc < THRESHOLD_BRAKING) && (!*enable_breaking) && (*spd < MAX_SPD_ENABLED)
            && (*spd > MIN_SPD_ENABLED)) {
            *enable_breaking = true;
            if (ttc < (THRESHOLD_BRAKING / 2.0)) {
                *lk_seatbelt = true;  
                *lk_doors = false;    
            }
        }       
    } else {
        *alarm_cluster = false;
        *enable_breaking = false;
    }
}