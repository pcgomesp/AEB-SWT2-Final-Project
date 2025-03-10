#include <stdio.h>
#include <pthread.h>
#include <mqueue.h>
#include <unistd.h>
#include <signal.h>
#include "mq_utils.h"
#include "constants.h"

int main()
{
    int brake_pedal;
    int speed;

    mqd_t mq_receiver = create_mq(MQ_NAME);

    signal(SIGINT, close_mq);

    while(1)
    {
        read_mq(mq_receiver, &brake_pedal, &speed);
        printf("Brake pedal: %d, Speed: %d\n", brake_pedal, speed);
        sleep(1);
    }

    return 0;
}