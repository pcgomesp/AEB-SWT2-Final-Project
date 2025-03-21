#ifndef LEITURA_ARQUIVO_H
#define LEITURA_ARQUIVO_H

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <pthread.h>
#include "constants.h"
#include <stdbool.h>

#include "../inc/sensors_input.h"

//#include "sensors_input.h"


// // Estrutura da mensagem a ser enviada pela fila
// typedef struct {
//     int tempo;
//     double distancia;
//     bool obstacle;
//     double speed;
//     bool brake;
//     bool accelerator;
//     bool aeb_on_off;
// } DadosMensagem;

// // Variável global para armazenar os dados lidos
// extern DadosMensagem dados;
// extern pthread_mutex_t dados_mutex;  // Declaração do mutex

// // Funções do módulo
// void* leitura_thread(void* arg);

//jogando aqui por enquanto para testar
//void read_sensor_data(const char *filename);

#endif