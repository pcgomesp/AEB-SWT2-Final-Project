#ifndef FILE_READER_H
#define FILE_READER_H

#include <stdio.h>
#include "sensors_input.h"

// Função para abrir o arquivo e pular o cabeçalho
FILE* open_file(const char* filename);

// Função para ler uma linha do arquivo
int read_sensor_data(FILE *file, sensors_input_data *sensor_data);

#endif