/**
 * @file can_utils.c
 * @brief Utilitários para manipulação de mensagens CAN, conforme especificação do DBC especificado.
 *
 * Este arquivo contém uma função para exibição de mensagens CAN, para ajudar no debugging do código em desenvolvimento.
 */

#include <stdio.h>
#include "dbc.h"

/**
 * @brief Imprime o identificador e data frame de uma mensagem CAN, seguindo a abstração de DBC.h.
 *
 * Exibe o identificador da mensagem CAN e os dados contidos formato hexadecimal.
 *
 * @param msg Ponteiro para a estrutura da mensagem CAN a ser impressa.
 */
void print_can_msg(const can_msg *msg) {
    printf("Identifier: %08X\n", msg->identifier);
    printf("Data Frame: ");
    for (int i = 0; i < 8; i++) {
        printf("%02X ", msg->dataFrame[i]);
    }
    printf("\n");
}