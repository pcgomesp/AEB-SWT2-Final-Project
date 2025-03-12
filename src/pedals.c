#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "constants.h"
#include "mq_utils.h"
#include "leitura.h"

extern DadosMensagem dados; // Declaração da variável global

int main()
{
    pthread_t t_leitura;
    pthread_create(&t_leitura, NULL, leitura_thread, NULL);
    
    mqd_t mq_sender = open_mq(MQ_NAME);

    int brake = 0;
    while (1)
    {
        brake = dados.distancia;
        char buffer[MQ_MAX_MSG_SIZE];
        sprintf(buffer, "B: %d", brake);

        printf("%s\n", buffer);

        write_mq(mq_sender, buffer);
        sleep(1);
    }
    pthread_join(t_leitura, NULL);
    return 0;
}
