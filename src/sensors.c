#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "constants.h"
#include "mq_utils.h"

int main()
{
    mqd_t mq_sender = open_mq(SENSORS_MQ);
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