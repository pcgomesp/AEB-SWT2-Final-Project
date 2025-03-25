#ifndef LOG_UTILS_H
#define LOG_UTILS_H

#include <stdio.h>
#include <actuators.h>
#include <stdint.h>

// Função para registrar eventos no log
void log_event(const char *id_aeb, uint32_t event_id, actuators_abstraction actuators);

#endif