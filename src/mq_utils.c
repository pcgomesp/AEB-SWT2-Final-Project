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
    mqd_t mqd = mq_open(mq_name, O_RDWR | O_CREAT | O_NONBLOCK, QUEUE_PERMISSIONS, &attr);
    if (mqd == (mqd_t)-1)
    {
        perror("Error creating message queue");
        exit(1);
    }

    printf("Queue %s created\n", mq_name);

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

void close_mq(mqd_t mqd, char *mq_name)
{
    printf("Closing %s message queue\n", mq_name);
    if (mq_close(mqd) == -1)
    {
        perror("Error closing message queue");
        exit(1);
    }
    if (mq_unlink(mq_name) == -1)
    {
        perror("Error unlinking message queue");
        exit(1);
    }
}

int read_mq(mqd_t mq_receiver, char* buffer)
{
    if (mq_receive(mq_receiver, buffer, MQ_MAX_MSG_SIZE, NULL) == (mqd_t)-1)
    {
        perror("Error receiving message");
        return -1;
    }
    return 0;
}

int write_mq(mqd_t mq_sender, char *msg)
{
    if (mq_send(mq_sender, msg, strlen(msg) + 1, 0) == -1)
    {
        perror("Error sending message. Message queue is full");
        return -1;
    }
    return 0;
}