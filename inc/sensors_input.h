/**
 * @file sensors_input.h
 * @brief Defines the data structure for sensor inputs to the AEB system.
 *
 * This file declares the structure that holds sensor input values required
 * by the AEB (Autonomous Emergency Braking) system. These values represent
 * real-time data from vehicle sensors used for decision-making.
 *
 * @details
 * - Contains relative velocity and acceleration between vehicle and detected object.
 * - Flags for obstacle presence and system activation status.
 * - Pedal input statuses from driver (brake and accelerator).
 * - Contais gear information - if the vehicle is in reverse or forward mode
 */

#ifndef SENSORS_INPUT_H
#define SENSORS_INPUT_H

#include <stdbool.h>

typedef struct {
    double relative_velocity;
    int has_obstacle;
    double obstacle_distance;
    int brake_pedal;
    int accelerator_pedal;
    int on_off_aeb_system;
    int reverseEnabled;
    double relative_acceleration;
} sensors_input_data;



#endif