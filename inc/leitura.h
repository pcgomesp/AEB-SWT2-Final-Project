#ifndef LEITURA_ARQUIVO_H
#define LEITURA_ARQUIVO_H

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <pthread.h>
#include "constants.h"

// Estrutura da mensagem a ser enviada pela fila
typedef struct {
    int tempo;
    int distancia;
} DadosMensagem;

// Variável global para armazenar os dados lidos
extern DadosMensagem dados;
extern pthread_mutex_t dados_mutex;  // Declaração do mutex

// Funções do módulo
void* leitura_thread(void* arg);

// Estrutura da mensagem estilo CAN a ser enviada para a fila
typedef struct {
    unsigned char identifier[8];
    unsigned char dataFrame[8];
} can_msg;

#endif