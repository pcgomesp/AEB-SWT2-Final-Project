/**
 * @file dbc.c
 * @brief Utilities for handling CAN messages, according to the DBC specification.
 *
 * This file contains a function for displaying CAN messages,
 * to help with debugging the code under development.
 */

#include <stdio.h>
#include "dbc.h"

/**
 * @brief Prints the identifier and data frame of a CAN message, following the DBC.h abstraction.
 *
 * Displays the CAN message identifier and the data contained in hexadecimal format.
 *
 * @param msg Pointer to the structure of the CAN message to be printed.
 */
void print_can_msg(const can_msg *msg)
{
    printf("Identifier: %08X\n", msg->identifier);
    printf("Data Frame: ");
    for (int i = 0; i < 8; i++)
    {
        printf("%02X ", msg->dataFrame[i]);
    }
    printf("\n");
}