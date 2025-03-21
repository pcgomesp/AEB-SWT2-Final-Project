#include <stdio.h>
#include <stdlib.h>

#include "../inc/sensors_input.h"
sensors_input_data* read_sensor_data(const char *filename) { //antes era void
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Erro ao abrir o arquivo");
        exit(EXIT_FAILURE);
    }
    
    char header[100]; // Para armazenar a linha do cabeçalho
    fgets(header, sizeof(header), file);
    


    //sensors_input_data sensor_data;

    // Aloca memória para a struct
    sensors_input_data *sensor_data = (sensors_input_data*)malloc(sizeof(sensors_input_data));
    if (sensor_data == NULL) {
        perror("Erro ao alocar memória");
        fclose(file);
        exit(EXIT_FAILURE);
    }
    
    // Lê os dados da primeira linha
    if (fscanf(file, "%lf %d %lf %d %d %d", 
                  &sensor_data->obstacle_distance, 
                  &sensor_data->has_obstacle, 
                  &sensor_data->vehicle_velocity, 
                  &sensor_data->brake_pedal, 
                  &sensor_data->accelerator_pedal, 
                  &sensor_data->on_off_aeb_system) == 6)
    
    // Itera sobre todas as linhas do arquivo
    // if (fscanf(file, "%lf %d %lf %d %d %d",  //se eu deixar um if, vai pegar so a prmira linha
    //               &sensor_data.obstacle_distance, 
    //               &sensor_data.has_obstacle, 
    //               &sensor_data.vehicle_velocity, 
    //               &sensor_data.brake_pedal, 
    //               &sensor_data.accelerator_pedal, 
    //               &sensor_data.on_off_aeb_system) == 6) {
        
    //     // Exibe os dados lidos
    //     printf("Velocidade do Veículo: %.2f m/s\n", sensor_data.vehicle_velocity);
    //     printf("Há Obstáculo: %d\n", sensor_data.has_obstacle);
    //     printf("Distância do Obstáculo: %.2f m\n", sensor_data.obstacle_distance);
    //     printf("Pedal do Freio: %d\n", sensor_data.brake_pedal);
    //     printf("Pedal do Acelerador: %d\n", sensor_data.accelerator_pedal);
    //     printf("Sistema AEB Ligado: %d\n", sensor_data.on_off_aeb_system);
    //     printf("-------------------------\n");
    // }
    
    fclose(file);
    return sensor_data;
}

int main() {
    //read_sensor_data("./cts/cenario.txt");


    //sensors_input_data sensor_data = read_sensor_data("./cts/cenario.txt");


    sensors_input_data *sensor_data = read_sensor_data("./cts/cenario.txt");

    // Exibe os dados lidos
    printf("Velocidade do Veículo: %.2f m/s\n", sensor_data->vehicle_velocity);
    printf("Há Obstáculo: %d\n", sensor_data->has_obstacle);




    return 0;
}
