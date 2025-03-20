#include <stdio.h>
#include "dbc.h"

void print_can_msg(const can_msg *msg) {
    printf("Identifier: ");
    for (int i = 0; i < 4; i++) {
        printf("%02X ", msg->identifier[i]);
    }
    printf("\nData Frame: ");
    for (int i = 0; i < 8; i++) {
        printf("%02X ", msg->dataFrame[i]);
    }
    printf("\n");
}