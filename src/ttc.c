#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "ttc.h"

double calculate_ttc_placeholder(double distance, double velocity_difference){
    if (velocity_difference <= 0) {
        fprintf(stderr, "Erro: A diferença de velocidade deve ser maior que zero.\n");
        return -1.0; // Valor indicativo de erro
    }
    return (distance / (velocity_difference/3.6));
}