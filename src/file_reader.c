/**
 * @file file_reader.c
 * @brief Module responsible for handling input file operations related to sensor data.
 * 
 * This module provides functions to open a file containing sensor input values and read its 
 * contents into a structured format. It skips the header line and parses sensor data values 
 * for later use in simulation or testing environments.
 */

#include "file_reader.h"
#include <stdlib.h>

/**
 * @brief Opens a file for reading and skips the first line (header).
 * 
 * @param filename Name of the file to be opened.
 * @return FILE* Pointer to the opened file.
 * @note If the file cannot be opened, the program exits with an error.
 * \anchor open_file
 */

FILE* open_file(const char* filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Erro ao abrir o arquivo");
        exit(EXIT_FAILURE);
    }

    // Skip header
    char header[100];
    fgets(header, sizeof(header), file);

    return file;
}


/**
 * @brief Reads a line from the file and fills the sensor data structure.
 * 
 * @param file Pointer to the opened file.
 * @param sensor_data Pointer to the structure where the data will be stored.
 * @return int Returns 1 if the reading is successful, 0 otherwise.
 * 
 * @note The expected file format will follow this label formact:
            Distance(m) Obstacle Speed(m/s) Brake Accelerator AEB_on_off Reverse
    \anchor read_sensor_data
 */
int read_sensor_data(FILE *file, sensors_input_data *sensor_data) {
    return fscanf(file, "%lf %d %lf %d %d %d %d %lf", 
                  &sensor_data->obstacle_distance, 
                  &sensor_data->has_obstacle, 
                  &sensor_data->relative_velocity, 
                  &sensor_data->brake_pedal, 
                  &sensor_data->accelerator_pedal, 
                  &sensor_data->on_off_aeb_system,
                  &sensor_data->reverseEnabled,
                  &sensor_data->relative_acceleration
                ) == 8;
}