#ifndef LEITURA_ARQUIVO_H
#define LEITURA_ARQUIVO_H

#include <stdio.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <pthread.h>

// definindo a memoria e o semaforo
#define SHM_NAME "/mem_compartilhada"
#define SEM_NAME "/sem_mem"

typedef struct {
    int tempo;
    int distancia;
    int velocidade;
} DadosCompartilhados;

// Variáveis globais do módulo
extern DadosCompartilhados* dados;
extern sem_t* sem_mem;

extern DadosCompartilhados* dados;
extern sem_t* sem_mem;

// funcoes do modulo
//lembrar de depois passar pro ingles
void inicializar_memoria_compartilhada();
void liberar_memoria_compartilhada();
void* leitura_thread(void* arg);

#endif
