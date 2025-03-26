#ifndef SENSORS_INPUT_H
#define SENSORS_INPUT_H

#include <stdbool.h>

typedef struct {
    double vehicle_velocity;
    int has_obstacle;
    double obstacle_distance;
    int brake_pedal;
    int accelerator_pedal;
    int on_off_aeb_system;
    int reverseEnabled;
} sensors_input_data;



#endif