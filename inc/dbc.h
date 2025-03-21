#ifndef DBC_H
#define DBC_H

#include <stdint.h>

// Identifiers, according to the dbc file in the requirements specification
#define ID_PEDALS       0x18FEF100
#define ID_SPEED_S      0x18FFFD64
#define ID_OBSTACLE_S   0x0CFFB027
#define ID_CAR_C        0x0CFFAF27
#define ID_AEB_S        0x18FFA027

// #define ID_PEDALS       {0x18, 0xFE, 0xF1, 0x00}
// #define ID_SPEED_S      {0x18, 0xFF, 0xFD, 0x64}
// #define ID_OBSTACLE_S   {0x0C, 0xFF, 0xB0, 0x27}
// #define ID_CAR_C        {0x0C, 0xFF, 0xAF, 0x27}
// #define ID_AEB_S        {0x18, 0xFF, 0xA0, 0x27}

// left most: least significant // right most: most significant
#define BASE_DATA_FRAME {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}

#define MAX_SPEED_S     251
#define MAX_OBSTACLE_S  150

#define RES_SPEED_S     (1.0/256.0)
#define RES_OBSTACLE_S  (1.0/20.0)

// Estrutura da mensagem estilo CAN a ser enviada para a fila
typedef struct {
    //unsigned char identifier[8];
    uint32_t identifier;
    unsigned char dataFrame[8];
} can_msg;

void print_can_msg(const can_msg *msg);

#endif