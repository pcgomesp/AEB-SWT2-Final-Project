/**
 * @file mq_utils.c
 * @brief Utilities for handling POSIX message queues.
 *
 * This file contains functions for creating, opening, closing,
 * reading, and writing POSIX message queues.
 */

#include "mq_utils.h"
#include "constants.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define QUEUE_PERMISSIONS 0660

/**
 * @brief Get the default attributes for a message queue.
 *
 * @return A mq_attr structure configured with default values.
 */
struct mq_attr get_mq_attr()
{
    struct mq_attr attr;
    attr.mq_flags = O_NONBLOCK;
    attr.mq_curmsgs = 0;
    attr.mq_maxmsg = MQ_MAX_MESSAGES;
    attr.mq_msgsize = MQ_MAX_MSG_SIZE;
    return attr;
}

/**
 * @brief Create a POSIX message queue.
 *
 * @param mq_name Name of the message queue to be created.
 * @return Message queue identifier (mqd_t).
 */
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

/**
 * @brief Abre uma fila de mensagens POSIX existente.
 *
 * @param mq_name Nome da fila de mensagens a ser aberta.
 * @return Identificador da fila de mensagens (mqd_t).
 */
 /**
 * @brief Open a POSIX message queue.
 *
 * @param mq_name Name of the message queue to be opened.
 * @return Message queue identifier (mqd_t).
 */
mqd_t open_mq(char *mq_name)
{
    struct mq_attr attr = get_mq_attr();
    mqd_t mqd = mq_open(mq_name, O_RDWR | O_NONBLOCK, QUEUE_PERMISSIONS, &attr);
    if (mqd == (mqd_t)-1)
    {
        perror("Error opening message queue");
        exit(1);
    }
    return mqd;
}

/**
 * @brief Close and unlink the specified POSIX message queue.
 *
 * @param mqd Message queue identifier.
 * @param mq_name Name of the message queue to be closed.
 */
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

/**
 * @brief Read a message from a POSIX message queue.
 *
 * @param mq_receiver Identifier of the message queue from which the message will be read.
 * @param msg_read Pointer to the structure where the read message will be stored, in can_msg struct type.
 * @return 0 on success, -1 on failure.
 */
int read_mq(mqd_t mq_receiver, can_msg *msg_read)
{
    char buffer[MQ_MAX_MSG_SIZE];
    if (mq_receive(mq_receiver, buffer, MQ_MAX_MSG_SIZE, NULL) == (mqd_t)-1)
    {
        // perror("Error receiving message");
        return -1;
    }
    memcpy(msg_read, buffer, MQ_MAX_MSG_SIZE);
    return 0;
}

/**
 * @brief Write a message from a POSIX message queue.
 *
 * @param mq_receiver Identifier of the message queue from which the message will be read.
 * @param msg_read Pointer to the can_msg struct type used to write a message in the MQ POSIX format.
 * @return 0 on success, -1 on failure.
 */
int write_mq(mqd_t mq_sender, can_msg *msg)
{
    char buffer[MQ_MAX_MSG_SIZE];
    memcpy(buffer, msg, MQ_MAX_MSG_SIZE);
    if (mq_send(mq_sender, buffer, MQ_MAX_MSG_SIZE, 0) == -1)
    {
        perror("Error sending message. Message queue is full");
        return -1;
    }
    return 0;
}