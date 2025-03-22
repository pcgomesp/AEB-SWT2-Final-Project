#include "../inc/file_reader.h"
#include <stdlib.h>

FILE* open_file(const char* filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Erro ao abrir o arquivo");
        exit(EXIT_FAILURE);
    }

    // Pula o cabeçalho do arquivo
    char header[100];
    fgets(header, sizeof(header), file);

    return file;
}

int read_sensor_data(FILE *file, sensors_input_data *sensor_data) {
    return fscanf(file, "%lf %d %lf %d %d %d", 
                  &sensor_data->obstacle_distance, 
                  &sensor_data->has_obstacle, 
                  &sensor_data->vehicle_velocity, 
                  &sensor_data->brake_pedal, 
                  &sensor_data->accelerator_pedal, 
                  &sensor_data->on_off_aeb_system) == 6;
}