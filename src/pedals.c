#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../inc/constants.h"
#include "../inc/mq_utils.h"

int main()
{
    mqd_t mq_sender = open_mq(MQ_NAME);
    int brake = 0;
    while(1)
    {
        brake += rand() % 20 - 10;
        char buffer[MQ_MAX_MSG_SIZE];
        sprintf(buffer, "B: %d", brake);

        printf("%s\n",buffer);
        
        write_mq(mq_sender, buffer);
        sleep(1);
    }

    return 0;
}