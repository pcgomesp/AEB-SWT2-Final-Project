// Necessary libraries
#include "../inc/ttc_control.h"

/**
 * @brief Calculate acceleration based on speed and time difference.
 *
 * This function calculates the acceleration in m/s² based on the difference in speed and time. 
 * It uses the `clock_gettime` function to measure the time difference between consecutive calls 
 * and calculates acceleration as the change in speed divided by the elapsed time.
 * 
 * @param spd Current speed in km/h.
 * 
 * @return The acceleration in m/s². If the function is called for the first time or there is
 *         insufficient time difference between calls, it returns 0.0.
 */
float accel_calc(float spd) {
    static float prev_spd = 0.0;
    static struct timespec start_time = {0, 0};
    struct timespec current_time;
    double elapsed_time;
    float accel = 0.0;

    clock_gettime(CLOCK_REALTIME, &current_time);

    if (prev_spd == 0.0) {
        prev_spd = spd;
        start_time = current_time;
        return 0.0;
    }

    else {
        elapsed_time = (double)(current_time.tv_sec - start_time.tv_sec) 
                        + (double)(current_time.tv_nsec - start_time.tv_nsec) / 1e9;
        if (elapsed_time < 0.01) return 0.0; // To avoid division by zero
        
        accel = ((spd - prev_spd) / 3.6) / elapsed_time; // acceleration in m/s^2
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
 * simply calculates the time by dividing the relative distance by the relative speed.
 *
 * @param dis_rel The relative distance between the objects in meters.
 * @param spd_rel The relative speed between the objects in km/h.
 * 
 * @return The time to collision in seconds. If no real solution is found (i.e., negative discriminant),
 *         the function returns -1.0. If there is no relative acceleration, the time to collision is calculated
 *         as distance divided by speed.
 */
float ttc_calc(float dis_rel, float spd_rel) {
    // Quadratic equation coefficients - UVM (Uniformly Variable Motion)
    float a, b, c, ttc, delta;
    
    a = accel_calc(spd_rel);
    b = spd_rel / 3.6;
    c = dis_rel;

    if (a == 0) return ttc = dis_rel / spd_rel;

    // Calculating the discriminant
    delta = b * b + 2 * a * c;
    if (delta < 0) return -1.0; // No real roots

    else if (delta == 0) return -b / a;

    else {
        // calculating the root
        ttc = (-b + sqrt(delta)) / a;

        // returning the positive root
        return ttc;
    }
}

/**
 * @brief Decides whether to trigger the Autonomous Emergency Braking (AEB) system based on TTC.
 *
 * This function calculates the Time-to-Collision (TTC) using the relative distance and speed 
 * between objects, and decides whether the AEB system should activate the alarm or apply the brakes.
 * The decision is made based on predefined TTC thresholds:
 * - If the TTC is below the `threshold_alarm` (e.g., 2 seconds), an alarm is triggered.
 * - If the TTC is below the `threshold_braking` (e.g., 1 second), the braking system is activated.
 * 
 * The AEB system and the alarm are only triggered if the `enable_aeb` flag is set to true.
 *
 * @param enable_aeb Pointer to a flag indicating whether the AEB system is enabled [SwR-5]. If true, the system can take action.
 * @param alarm_cluster Pointer to a flag that will be set to true if the alarm should be triggered [SwR-2]. It is set to false when no action is needed.
 * @param enable_breaking Pointer to a flag that will be set to true if the braking system should be enabled [SwR-3]. It is set to false when no braking action is required.
 * @param spd Pointer to the current vehicle speed (in km/h). This value is used to calculate the relative speed.
 * @param dist Pointer to the relative distance to the object ahead (in meters). This value is used to calculate the TTC.
 * 
 * @note The function assumes that `spd` and `dist` are updated regularly, reflecting the current relative speed and distance to the object ahead.
 */
void aeb_control(bool *enable_aeb, bool *alarm_cluster, bool *enable_breaking, 
                 float *spd, float *dist) {
    float ttc;
    
    // Define the critical TTC thresholds (in seconds) below which AEB will be triggered
    const float threshold_alarm = 2.0;  /// [SwR-2] < Threshold for triggering the alarm (TTC < 2.0 seconds)
    const float threshold_braking = 1.0; /// [SwR-3] < Threshold for triggering the braking system (TTC < 1.0 second)
    
    // Calculate TTC using the relative distance and speed
    ttc = ttc_calc(*dist, *spd);
    
    // If AEB is enabled and TTC is below the threshold, trigger the braking action
    if (*enable_aeb && ttc > 0.0 && ttc < threshold_alarm) {
        // Set the alarm flag to true if TTC is below the alarm threshold
        *alarm_cluster = true;
        
        // Trigger the braking system if TTC is below the braking threshold
        if (ttc < threshold_braking) {
            *enable_breaking = true;
        }       
    } else {
        // Reset the alarm and braking flags if TTC is above the threshold or AEB is disabled
        *alarm_cluster = false;
        *enable_breaking = false;
    }
}
