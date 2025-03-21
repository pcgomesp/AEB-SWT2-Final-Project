#include "leitura.h"
#include "mq_utils.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

extern mqd_t mq;  // Declaração da variável global
DadosMensagem dados;  // Definição da variável global

// Thread de leitura do arquivo
void* leitura_thread(void* arg) {
    FILE* arquivo = fopen("../cts/cenario.txt", "r");
    if (!arquivo) {
        perror("Erro ao abrir o arquivo cenario.txt");
        return NULL;
    }

    char buffer[100];
    fgets(buffer, sizeof(buffer), arquivo); // Ignora o cabeçalho

    while (fscanf(arquivo, "%d %f %d %f %d %d %d", &dados.tempo, &dados.distancia, &dados.obstacle, 
    &dados.speed, &dados.brake, &dados.accelerator, &dados.aeb_on_off) == 7) {
        //printf("Lido: Tempo = %d, Distância = %d\n", dados.tempo, dados.distancia);
        usleep(dados.tempo * 1000);
    }

    fclose(arquivo);
    printf("Leitura do arquivo concluída.\n");
    return NULL;
}