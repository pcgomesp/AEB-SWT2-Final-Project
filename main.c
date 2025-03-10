#include "features/leitura_arquivo.h"
#include <pthread.h>


int main() {
    inicializar_memoria_compartilhada();

    pthread_t t_leitura;
    pthread_create(&t_leitura, NULL, leitura_thread, NULL); // Criação da thread de leitura
    pthread_join(t_leitura, NULL);

    //aqui vai o restante dos codigod, que farão o processamento desses dados lidos

    liberar_memoria_compartilhada();

    return 0;
}
