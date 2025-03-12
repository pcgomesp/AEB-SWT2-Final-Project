#include <stdio.h>
#include <unistd.h>
#include <mqueue.h>
#include "mq_utils.h"
#include "constants.h"

int main()
{
    mqd_t mq_receiver = open_mq(ACTUATORS_MQ);
    char buffer[MQ_MAX_MSG_SIZE];
    while (1)
    {
        read_actuators_mq(mq_receiver, buffer);
        printf("Received message: <%s>\n", buffer);
        sleep(1);
    }

    return 0;
}