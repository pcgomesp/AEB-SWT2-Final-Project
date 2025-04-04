#ifndef DBC_H
#define DBC_H

#include <stdint.h>

// Calibration values [SwR-13]
// Identifiers, according to the dbc file in the requirements specification
#define ID_PEDALS 0x18FEF100
#define ID_SPEED_S 0x18FFFD64
#define ID_OBSTACLE_S 0x0CFFB027
#define ID_CAR_C 0x0CFFAF27
#define ID_AEB_S 0x18FFA027
#define ID_EMPTY 0x00000000

// left most: least significant, right most: most significant
#define BASE_DATA_FRAME {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}

#define MAX_SPEED_S 251.0
#define MAX_OBSTACLE_S 300.0
#define MAX_ACCELERATION_S 12.5
#define MIN_ACCELERATION_S -12.5

#define RES_SPEED_S (1.0 / 256.0)
#define RES_OBSTACLE_S (1.0 / 20.0)
//#define RES_ACCELERATION_S 0.001
#define RES_ACCELERATION_S (1.0 / 1000.0)

#define OFFSET_ACCELERATION_S -12500

// Struct to simulate a CAN message
typedef struct
{
    uint32_t identifier;
    unsigned char dataFrame[8];
} can_msg;

void print_can_msg(const can_msg *msg);

#endif