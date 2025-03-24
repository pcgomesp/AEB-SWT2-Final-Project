#ifndef CONSTANTS_H
#define CONSTANTS_H

#define SENSORS_MQ "/mq_aeb_sensors"
#define ACTUATORS_MQ "/mq_aeb_actuators"
#define MQ_MAX_MESSAGES 10
#define MQ_MAX_MSG_SIZE 16

#define SHM_NAME "/shm_aeb"
#define SEM_NAME "/sem_aeb"
#define SHM_PERMISSIONS 0666

// Calibration values

// Define the critical TTC thresholds (in seconds) below which AEB will be triggered
#define THRESHOLD_ALARM 2.0 /// [SwR-2] < Threshold for triggering the alarm (TTC < 2.0 seconds)
#define THRESHOLD_BRAKING 1.0 /// [SwR-3] < Threshold for triggering the braking system (TTC < 1.0 second)

#endif