#include <stdio.h>
#include <mqueue.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include "mq_utils.h"
#include "constants.h"
#include "leitura.h"

mqd_t sensors_mq;

int main()
{
    printf("Main process PID: %d\n", getpid());

    //Initialize resources

    //Create processes

    // Read data from file

    signal(SIGINT, terminate_execution);

    return EXIT_SUCCESS;
}

void terminate_execution(int sig)
{
    printf("Closing message queue\n");
    close_mq(sensors_mq, SENSORS_MQ);

    printf("Closing shared memory\n");
    //close_shm();
    exit(0);
}