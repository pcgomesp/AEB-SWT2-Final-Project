#include "../inc/leitura.h"
#include "../inc/mq_utils.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>




extern mqd_t mq;  // Declaração da variável global
//DadosMensagem dados;  // Definição da variável global

// Thread de leitura do arquivo
// void* leitura_thread(void* arg) {
//     FILE* arquivo = fopen("./cts/cenario.txt", "r");
//     if (!arquivo) {
//         perror("Erro ao abrir o arquivo cenario.txt");
//         return NULL;
//     }

//     char buffer[100];
//     fgets(buffer, sizeof(buffer), arquivo); // Ignora o cabeçalho

//     int obstacle, brake, accelerator, aeb_on_off;
//     while (fscanf(arquivo, "%d %lf %d %lf %d %d %d", &dados.tempo, &dados.distancia, &obstacle, 
//     &dados.speed, &brake, &accelerator, &aeb_on_off) == 7) {
//         dados.obstacle = obstacle;
//         dados.brake = brake;
//         dados.accelerator = accelerator;
//         dados.aeb_on_off = aeb_on_off;
//         //printf("Lido: Tempo = %d, Distância = %d\n", dados.tempo, dados.distancia);
//         usleep(dados.tempo * 1000);
//     }

//     fclose(arquivo);
//     printf("Leitura do arquivo concluída.\n");
//     return NULL;
// }


extern sensors_input_data sensorsData;  // Declaração da variável global

void leitura_arquivo(const char* filename) {
    FILE* arquivo = fopen(filename, "r");
    if (!arquivo) {
        perror("Erro ao abrir o arquivo cenario.txt");
        return;
    }

    char buffer[100];
    fgets(buffer, sizeof(buffer), arquivo); // Ignora o cabeçalho

    int has_obstacle, brake_pedal, accelerator_pedal, on_off_aeb_system;
    while (fscanf(arquivo, "%lf %d %lf %d %d %d", &sensorsData.vehicle_velocity, &has_obstacle, 
    &sensorsData.obstacle_distance, &brake_pedal, &accelerator_pedal, &on_off_aeb_system) == 6) {
        sensorsData.has_obstacle = has_obstacle;
        sensorsData.brake_pedal = brake_pedal;
        sensorsData.accelerator_pedal = accelerator_pedal;
        sensorsData.on_off_aeb_system = on_off_aeb_system;
        printf("Lido: Velocidade = %lf, Obstacle = %d, Distância = %lf, Brake = %d, Accelerator = %d, AEB On/Off = %d\n",
               sensorsData.vehicle_velocity, sensorsData.has_obstacle, sensorsData.obstacle_distance, sensorsData.brake_pedal, sensorsData.accelerator_pedal, sensorsData.on_off_aeb_system);
    }

    fclose(arquivo);
    printf("Leitura do arquivo concluída.\n");
}