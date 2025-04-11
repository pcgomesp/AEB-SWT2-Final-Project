#ifndef ACTUATORS_H
#define ACTUATORS_H

#include <stdbool.h>
#include "dbc.h"

typedef struct{
    bool belt_tightness;
    bool door_lock;
    bool should_activate_abs;
    bool alarm_led;
    bool alarm_buzzer;
} actuators_abstraction;

void actuatorsTranslateCanMsg(can_msg captured_frame);
void updateInternalActuatorsState(can_msg captured_frame);
void print_info_output();

#endif