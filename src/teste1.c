#include <stdio.h>
#include <stdlib.h>
#include "../inc/sensors_input.h"

typedef struct Node {
    sensors_input_data data;
    struct Node* next;
} Node;

Node* read_sensor_data(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Erro ao abrir o arquivo");
        exit(EXIT_FAILURE);
    }
    
    char header[100]; // Para armazenar a linha do cabeçalho
    fgets(header, sizeof(header), file);

    Node* head = NULL;
    Node* tail = NULL;

    while (1) {
        sensors_input_data sensor_data;
        if (fscanf(file, "%lf %d %lf %d %d %d", 
                  &sensor_data.obstacle_distance, 
                  &sensor_data.has_obstacle, 
                  &sensor_data.vehicle_velocity, 
                  &sensor_data.brake_pedal, 
                  &sensor_data.accelerator_pedal, 
                  &sensor_data.on_off_aeb_system) != 6) {
            break;
        }

        Node* new_node = (Node*)malloc(sizeof(Node));
        if (new_node == NULL) {
            perror("Erro ao alocar memória");
            fclose(file);
            exit(EXIT_FAILURE);
        }
        new_node->data = sensor_data;
        new_node->next = NULL;

        if (head == NULL) {
            head = new_node;
            tail = new_node;
        } else {
            tail->next = new_node;
            tail = new_node;
        }
    }

    fclose(file);
    return head;
}

void free_sensor_data(Node* head) {
    Node* current = head;
    while (current != NULL) {
        Node* next = current->next;
        free(current);
        current = next;
    }
}

int main() {
    Node* sensor_data_list = read_sensor_data("cts/cenario.txt");

    Node* current = sensor_data_list;
    while (current != NULL) {
        sensors_input_data* sensor_data = &current->data;
        // Exibe os dados lidos
        printf("Velocidade do Veículo: %.2f m/s\n", sensor_data->vehicle_velocity);
        printf("Há Obstáculo: %d\n", sensor_data->has_obstacle);
        printf("Distância do Obstáculo: %.2f m\n", sensor_data->obstacle_distance);
        printf("Pedal do Freio: %d\n", sensor_data->brake_pedal);
        printf("Pedal do Acelerador: %d\n", sensor_data->accelerator_pedal);
        printf("Sistema AEB Ligado: %d\n", sensor_data->on_off_aeb_system);
        printf("-------------------------\n");

        current = current->next;
    }

    free_sensor_data(sensor_data_list);

    return 0;
}