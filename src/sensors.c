// Necessary libraries
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../inc/constants.h"
#include "../inc/mq_utils.h"

int main()
{
    mqd_t mq_sender = open_mq(MQ_NAME);
    int speed = 0;
    while(1)
    {
        speed += rand() % 20 - 10;
        char buffer[MQ_MAX_MSG_SIZE];
        sprintf(buffer, "S: %d", speed);
        printf("%s\n",buffer);

        write_mq(mq_sender, buffer);
        sleep(1);
    }

    return 0;
}