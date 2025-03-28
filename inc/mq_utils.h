#ifndef MQ_UTILS_H
#define MQ_UTILS_H
#include <mqueue.h>
#include "dbc.h"

struct mq_attr get_mq_attr();

mqd_t create_mq(char *mq_name);

mqd_t open_mq(char *mq_name);

void close_mq(mqd_t mqd, char *mq_name);

int read_mq(mqd_t mq_receiver, can_msg *msg_read);

int write_mq(mqd_t mq_sender, can_msg *msg);

#endif