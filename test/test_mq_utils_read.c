#include <stdio.h>
#include <sys/stat.h>
#define UNITY_VERBOSE_OUTPUT
#include "unity.h"
#include "mq_utils.h"
#include "constants.h"

char *mq_name = SENSORS_MQ;
mqd_t mqd;

void setUp()
{
    mqd = create_mq(mq_name);
}

void tearDown()
{
    close_mq(mqd, mq_name);
}

void test_open_mq()
{
    mqd_t mq_test = open_mq(mq_name);
    TEST_ASSERT_NOT_EQUAL(-1, mq_test);
}

void test_read_mq_empty_queue()
{
    can_msg msg_read;
    TEST_ASSERT_EQUAL(-1, read_mq(mqd, &msg_read));
}

void test_write_mq_full_queue()
{
    can_msg msg_to_write = {0};
    for (int i = 0; i < MQ_MAX_MESSAGES; i++)
    {
        write_mq(mqd, &msg_to_write);
    }
    TEST_ASSERT_EQUAL(-1, write_mq(mqd, &msg_to_write));
}

void test_read_and_write_mq_empty_can_msg()
{
    mqd_t mq_write = open_mq(mq_name);
    can_msg msg_to_write = {0};
    write_mq(mq_write, &msg_to_write);

    can_msg msg_read;
    read_mq(mqd, &msg_read);

    TEST_ASSERT_EQUAL(msg_to_write.identifier, msg_read.identifier);
}

void test_read_and_write_mq_valid_can_msg()
{
    mqd_t mq_write = open_mq(mq_name);
    can_msg msg_to_write = {
        .identifier = 12345,
        .dataFrame = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88}};
    write_mq(mq_write, &msg_to_write);

    can_msg msg_read;
    read_mq(mqd, &msg_read);

    TEST_ASSERT_EQUAL(msg_to_write.identifier, msg_read.identifier);
    for (int i = 0; i < 8; i++)
    {
        TEST_ASSERT_EQUAL(msg_to_write.dataFrame[i], msg_read.dataFrame[i]);
    }
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_open_mq);
    RUN_TEST(test_read_mq_empty_queue);
    RUN_TEST(test_write_mq_full_queue);
    RUN_TEST(test_read_and_write_mq_empty_can_msg);
    RUN_TEST(test_read_and_write_mq_valid_can_msg);
    return UNITY_END();
}