#include "mq_utils.h"
#include "constants.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define QUEUE_PERMISSIONS 0660

struct mq_attr get_mq_attr()
{
    struct mq_attr attr;
    attr.mq_flags = O_NONBLOCK;
    attr.mq_curmsgs = 0;
    attr.mq_maxmsg = MQ_MAX_MESSAGES;
    attr.mq_msgsize = MQ_MAX_MSG_SIZE;
    return attr;
}

mqd_t create_mq(char *mq_name)
{
    struct mq_attr attr = get_mq_attr();
    mqd_t mqd = mq_open(mq_name, O_RDONLY | O_CREAT | O_NONBLOCK, QUEUE_PERMISSIONS, &attr);
    if (mqd == (mqd_t)-1)
    {
        perror("Error creating message queue");
        exit(1);
    }
    return mqd;
}

mqd_t open_mq(char *mq_name)
{
    struct mq_attr attr = get_mq_attr();
    mqd_t mqd = mq_open(mq_name, O_RDWR, QUEUE_PERMISSIONS, &attr);
    if (mqd == (mqd_t)-1)
    {
        perror("Error opening message queue");
        exit(1);
    }
    return mqd;
}

void close_mq(mqd_t mqd)
{
    printf("Closing message queue\n");
    if (mq_close(mqd) == -1)
    {
        perror("Error closing message queue");
        exit(1);
    }
    if (mq_unlink(MQ_NAME) == -1)
    {
        perror("Error unlinking message queue");
        exit(1);
    }
    exit(0);
}

void read_mq(mqd_t mq_receiver, int *brake_pedal, int *speed)
{
    char mq_buffer[MQ_MAX_MSG_SIZE];
    if (mq_receive(mq_receiver, mq_buffer, MQ_MAX_MSG_SIZE, NULL) != (mqd_t)-1)
    {
        switch (mq_buffer[0])
        {
        case 'B':
            *brake_pedal = atoi(mq_buffer + 3);
            break;
        case 'S':
            *speed = atoi(mq_buffer + 3);
            break;
        default:
            break;        
        }
    }
}

void write_mq(mqd_t mq_sender, char *msg)
{
    if (mq_send(mq_sender, msg, strlen(msg) + 1, 0) == -1)
    {
        perror("Error sending message");
        exit(1);
    }
}