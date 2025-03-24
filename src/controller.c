#include <stdio.h>
#include <mqueue.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include "mq_utils.h"
#include "constants.h"

mqd_t sensors_mq;
mqd_t actuators_mq;

void terminate_execution(int sig)
{
    printf("Closing all message queues\n");
    close_mq(sensors_mq, SENSORS_MQ);
    close_mq(actuators_mq, ACTUATORS_MQ);
    exit(0);
}

void update_values(char *mq_buffer, int *brake_pedal, int *speed)
{
    switch(mq_buffer[0])
    {
        case 'B':
            sscanf(mq_buffer, "B: %d", brake_pedal);
            break;
        case 'S':
            sscanf(mq_buffer, "S: %d", speed);
            break;
    }
}

int main()
{
    printf("Controller process PID: %d\n", getpid());

    char mq_buffer[MQ_MAX_MSG_SIZE];
    int brake_pedal;
    int speed;

    sensors_mq = create_mq(SENSORS_MQ);
    actuators_mq = create_mq(ACTUATORS_MQ);

    signal(SIGINT, terminate_execution);

    while (1)
    {
        read_mq(sensors_mq, mq_buffer);
        update_values(mq_buffer, &brake_pedal, &speed);
        printf("Brake pedal: %d, Speed: %d\n", brake_pedal, speed);

        sprintf(mq_buffer, "Freio: %d", rand() % 100 + 1);
        printf("Sending message: %s\n", mq_buffer);
        write_mq(actuators_mq, mq_buffer);

        sleep(1);
    }

    return 0;
}