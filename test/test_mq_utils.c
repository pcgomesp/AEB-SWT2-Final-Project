#include <sys/stat.h>
#include "unity.h"
#include "mq_utils.h"
#include "constants.h"

void setUp()
{
    // set stuff up here
}

void tearDown()
{
    // clean stuff up here
}

void test_get_mq_attr()
{
    struct mq_attr attr = get_mq_attr();
    TEST_ASSERT_EQUAL(O_NONBLOCK, attr.mq_flags);
    TEST_ASSERT_EQUAL(0, attr.mq_curmsgs);
    TEST_ASSERT_EQUAL(MQ_MAX_MESSAGES, attr.mq_maxmsg);
    TEST_ASSERT_EQUAL(MQ_MAX_MSG_SIZE, attr.mq_msgsize);
}

void test_create_and_close_mq()
{
    char *mq_name = "/test_mq";
    struct stat buffer;

    int exists = stat("/dev/mqueue/test_mq", &buffer);
    TEST_ASSERT_EQUAL(-1, exists);

    mqd_t mqd = create_mq(mq_name);

    exists = stat("/dev/mqueue/test_mq", &buffer);
    TEST_ASSERT_EQUAL(0, exists);
    TEST_ASSERT_NOT_EQUAL((mqd_t)-1, mqd);

    close_mq(mqd, mq_name);
    exists = stat("/dev/mqueue/test_mq", &buffer);
    TEST_ASSERT_EQUAL(-1, exists);
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_get_mq_attr);
    RUN_TEST(test_create_and_close_mq);
    return UNITY_END();
}