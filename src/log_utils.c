/**
 * @file log_utils.c
 * @brief Utilitary file for event tracking in actuators abstraction.
 *
 * This file provides functionality to log events with timestamps 
 * and relevant actuator states.
 */
#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include "log_utils.h"


/**
 * @brief Logs an event to a file with a timestamp and actuator data.
 *
 * This function logs an event by appending it to a log file. It records the event ID, 
 * the timestamp (in milliseconds), and actuator states in a structured format.
 *
 * @param id_aeb Identifier for the AEB system.
 * @param event_id The unique event identifier (formatted as a hexadecimal string).
 * @param actuators Structure containing actuator state information.
 */

void log_event(const char *id_aeb, uint32_t event_id, actuators_abstraction actuators) {
    FILE *log_file = fopen("log/log.txt", "a");
    if (log_file == NULL) {
        perror("Error opening log file");
        return;
    }

    //Check if the file is empty
    fseek(log_file, 0, SEEK_END);
    if (ftell(log_file) == 0) {
        // If the file is empty, write the header
        fprintf(log_file, "ID_AEB | EVENT_ID | TIMESTAMP | MESSAGE | BELT_TIGHTNESS | DOOR_LOCK | ABS_ACTIVATION | ALARM_LED | ALARM_BUZZER\n");
    }
    fseek(log_file, 0, SEEK_END); // Move the file pointer to the end of the file

    // Getting the timestamp instead just the date, because it's more useful (miliseconds)
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    long timestamp_ms = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;

    // Convert id to hexadecimal string
    char event_id_str[9]; // 8 caracteres + '\0'
    snprintf(event_id_str, sizeof(event_id_str), "%08X", event_id);

    // Write in file in desired format
    fprintf(log_file, "%s | %s | %07ld | WARNING | %d | %d | %d | %d | %d\n",
            id_aeb,
            event_id_str,
            timestamp_ms,
            actuators.belt_tightness,
            actuators.door_lock,
            actuators.should_activate_abs,
            actuators.alarm_led,
            actuators.alarm_buzzer);

    fclose(log_file);
}
