#include <stdio.h>
#include <sys/stat.h>
#define UNITY_VERBOSE_OUTPUT
#include "unity.h"
#include "mq_utils.h"
#include "constants.h"

char *mq_name = SENSORS_MQ;
mqd_t mqd;

void setUp(void)
{
    mqd = create_mq(mq_name);
}

void tearDown(void)
{
    close_mq(mqd, mq_name);
}

void test_open_mq(void)
{
    mqd_t mq_test = open_mq(mq_name);
    TEST_ASSERT_NOT_EQUAL(-1, mq_test);
}

void test_read_and_write_mq(void)
{
    mqd_t mq_write = open_mq(mq_name);
    char *message = "Hello, World!";
    write_mq(mq_write, message);
    char buffer[MQ_MAX_MSG_SIZE];
    read_mq(mqd, buffer);
    TEST_ASSERT_EQUAL_STRING(message, buffer);
}

void test_read_mq_empty_queue(void)
{
    char buffer[MQ_MAX_MSG_SIZE] = "";
    TEST_ASSERT_EQUAL(-1, read_mq(mqd, buffer));
    TEST_ASSERT_EQUAL_STRING("", buffer);
}

void test_write_mq_full_queue(void)
{
    char *message = "Hello, World!";
    for (int i = 0; i < MQ_MAX_MESSAGES; i++)
    {
        write_mq(mqd, message);
    }
    TEST_ASSERT_EQUAL(-1, write_mq(mqd, message));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_open_mq);
    RUN_TEST(test_read_and_write_mq);
    RUN_TEST(test_read_mq_empty_queue);
    RUN_TEST(test_write_mq_full_queue);
    return UNITY_END();
}