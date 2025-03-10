#include "leitura_arquivo.h"

DadosCompartilhados* dados = NULL;
sem_t* sem_mem;

// 
void inicializar_memoria_compartilhada() {
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, sizeof(DadosCompartilhados));
    dados = mmap(0, sizeof(DadosCompartilhados), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    sem_mem = sem_open(SEM_NAME, O_CREAT, 0644, 1);
}

// Liberação da memória compartilhada
void liberar_memoria_compartilhada() {
    munmap(dados, sizeof(DadosCompartilhados));
    shm_unlink(SHM_NAME);
    sem_close(sem_mem);
    sem_unlink(SEM_NAME);
}

// Thread de leitura do arquivo
void* leitura_thread(void* arg) {
    FILE* arquivo = fopen("cenario.txt", "r"); //supondo que os cenarios de teste estejam num arquivo com esse nome
    //futuramente teremos que automatizar isso para ler varios arquivos, mas por agora fica assim para validar a funcionalidade do processamento
    if (!arquivo) {
        printf("Erro ao abrir o arquivo cenario.txt\n");
        return NULL;
    }

    char buffer[100];
    fgets(buffer, sizeof(buffer), arquivo); // Ignora o cabeçalho

    int tempo, distancia;
    while (fscanf(arquivo, "%d %d", &tempo, &distancia) == 2) { //por enquanto deixei somente esses dois, mas para criar basta adicionar mais variaveis
        usleep(tempo * 1000); // Converte milissegundos para microsegundos

        sem_wait(sem_mem);
        dados->tempo = tempo;
        dados->distancia = distancia; //por enquanto deixei somente esses dois, mas para criar basta adicionar mais variaveis. e tem que adicionar no struct tambem
        sem_post(sem_mem);
    }

    fclose(arquivo);
    return NULL;
}
