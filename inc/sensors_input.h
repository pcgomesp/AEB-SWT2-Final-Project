#ifndef SENSORS_INPUT_H
#define SENSORS_INPUT_H


#include <stdbool.h>

typedef struct {
    double  vehicle_velocity;
    bool    has_obstacle;
    double  obstacle_distance;
    bool    brake_pedal;
    bool    accelerator_pedal;
    bool    on_off_aeb_system;
} sensors_input_data;


#endif