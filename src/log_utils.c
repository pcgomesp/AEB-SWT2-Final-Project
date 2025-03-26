#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include "log_utils.h"

void log_event(const char *id_aeb, uint32_t event_id, actuators_abstraction actuators) {
    FILE *log_file = fopen("log/log.txt", "a");
    if (log_file == NULL) {
        perror("Error opening log file");
        return;
    }

    // Getting the timestamp instead just the date, because it's more useful (miliseconds)
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    long timestamp_ms = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;

    // Convert id to hexadecimal string
    char event_id_str[9]; // 8 caracteres + '\0'
    snprintf(event_id_str, sizeof(event_id_str), "%08X", event_id);

    // Write in file in desired format
    fprintf(log_file, "[%s][%s][T:%07ld] WARNING | %d | %d | %d | %d | %d\n",
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
