/**
 * @file constants.h
 * @brief Defines constants and calibration values for the AEB system.
 *
 * This file contains the definitions of constants, message queue names, shared memory names,
 * and calibration thresholds used throughout the AEB system. These constants are shared
 * across multiple modules, ensuring consistency in the system's behavior.
 *
 * @details
 * - Defines message queue names and sizes for inter-process communication.
 * - Specifies shared memory and semaphore names for synchronization.
 * - Includes calibration thresholds for triggering alarms and braking in the AEB system.
 * - Provides speed limits for enabling the AEB system.
 * - Implementates calibration values [SwR-13] (@ref SwR-13)
 *
 */

#ifndef CONSTANTS_H
#define CONSTANTS_H

#define SENSORS_MQ "/mq_aeb_sensors"
#define ACTUATORS_MQ "/mq_aeb_actuators"
#define MQ_MAX_MESSAGES 10
#define MQ_MAX_MSG_SIZE 12

#define SHM_NAME "/shm_aeb"
#define SEM_NAME "/sem_aeb"
#define SHM_PERMISSIONS 0666


// Define the critical TTC thresholds (in seconds) below which AEB will be triggered
//! Threshold for triggering the alarm (TTC < 2.0 seconds). [SwR-2] (@ref SwR-2)
#define THRESHOLD_ALARM 2.0 /// [SwR-2] (@ref SwR-2) < Threshold for triggering the alarm (TTC < 2.0 seconds)
//! Threshold for triggering the braking system (TTC < 1.0 second). [SwR-3] (@ref SwR-3)
#define THRESHOLD_BRAKING 1.0 /// [SwR-3] < Threshold for triggering the braking system (TTC < 1.0 second)
//! [Sys-F-9] < Maximum speed for which AEB is enabled in km/h
#define MAX_SPD_ENABLED 60.0 /// [Sys-F-9] < Maximum speed for which AEB is enabled in km/h
//! Minimum speed for which AEB is enabled in km/h [SwR-7] (@ref SwR-7)
#define MIN_SPD_ENABLED 10.0 /// [SwR-7] < Minimum speed for which AEB is enabled in km/h

#endif