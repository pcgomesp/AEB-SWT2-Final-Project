#ifndef MQ_UTILS_H
#define MQ_UTILS_H
#include <mqueue.h>

struct mq_attr get_mq_attr();

mqd_t create_mq(char *mq_name);

mqd_t open_mq(char *mq_name);

void close_mq(mqd_t mqd, char *mq_name);

// TODO: create a single function to read from both message queues

void read_sensors_mq(mqd_t mq_receiver, int *brake_pedal, int *speed);

void read_actuators_mq(mqd_t mq_receiver, char* buffer);

void write_mq(mqd_t mq_sender, char *msg);

#endif