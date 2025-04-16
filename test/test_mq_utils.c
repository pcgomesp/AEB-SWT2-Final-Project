#include <sys/stat.h>
#include <stdbool.h>
#include "unity.h"
#include "mq_utils.h"

static bool wrap_mq_open_fail = false;
static bool wrap_mq_unlink_fail = false;
static bool wrap_perror_called = false;

char *mq_name = "/test_mq"; // this could be any name
int mq_max_messages = 10;
mqd_t mqd;

mqd_t __real_mq_open(const char *name, int oflag, ...);
mqd_t __wrap_mq_open(const char *name, int oflag, ...)
{
    if (wrap_mq_open_fail)
        return (mqd_t)-1;
    return __real_mq_open(name, oflag);
}

int __real_mq_unlink(const char *name);
int __wrap_mq_unlink(const char *name)
{
    if (wrap_mq_unlink_fail)
        return -1;
    return __real_mq_unlink(name);
}

void __real_perror(const char *s);
void __wrap_perror(const char *s)
{
    wrap_perror_called = true;
    __real_perror(s);
}

void setUp()
{
    wrap_mq_open_fail = false;
    wrap_perror_called = false;
    close_mq(mqd, mq_name);
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
    TEST_ASSERT_EQUAL(10, attr.mq_maxmsg);
    TEST_ASSERT_EQUAL(12, attr.mq_msgsize);
}

/** @test */
void test_create_and_close_mq()
{
    char *mq_name = "/test_mq";
    struct stat buffer;

    int exists = stat("/dev/mqueue/test_mq", &buffer);
    TEST_ASSERT_EQUAL_MESSAGE(-1, exists, "Queue exists but should not");

    mqd_t mqd = create_mq(mq_name);

    exists = stat("/dev/mqueue/test_mq", &buffer);
    TEST_ASSERT_EQUAL_MESSAGE(0, exists, "Queue does not exist but should");
    TEST_ASSERT_NOT_EQUAL((mqd_t)-1, mqd);

    close_mq(mqd, mq_name);
    exists = stat("/dev/mqueue/test_mq", &buffer);
    TEST_ASSERT_EQUAL_MESSAGE(-1, exists, "Queue exists but should have been deleted");
}

void test_create_mq_fail()
{
    wrap_mq_open_fail = true;
    wrap_perror_called = false;

    mqd_t mqd = create_mq(mq_name);
    TEST_ASSERT_EQUAL((mqd_t)-1, mqd);
    TEST_ASSERT_TRUE(wrap_perror_called);
}

void test_close_unopened_mq_fail()
{
    mqd = (mqd_t)-1; // Uninitialized mqd
    struct stat buffer;

    int exists = stat("/dev/mqueue/test_mq", &buffer);
    TEST_ASSERT_EQUAL_MESSAGE(-1, exists, "Queue exists but should not");

    close_mq(mqd, mq_name);
    TEST_ASSERT_TRUE(wrap_perror_called);
}

void test_close_mq_fail_unlink()
{    
    mqd = create_mq(mq_name);
    
    wrap_mq_unlink_fail = true;
    close_mq(mqd, mq_name);
    TEST_ASSERT_TRUE(wrap_perror_called);

    wrap_mq_unlink_fail = false;
    mq_unlink(mq_name);
}

/** @test */
void test_open_mq()
{
    mqd = create_mq(mq_name);
    mqd_t mq_test = open_mq(mq_name);
    TEST_ASSERT_NOT_EQUAL(-1, mq_test);
    close_mq(mqd, mq_name);
}

/** @test */
void test_open_mq_fail()
{
    wrap_mq_open_fail = true;
    wrap_perror_called = false;

    mqd_t mq_test = open_mq(mq_name);
    TEST_ASSERT_EQUAL((mqd_t)-1, mq_test);
    TEST_ASSERT_TRUE(wrap_perror_called);
}

/** @test */
void test_read_mq_empty_queue()
{
    mqd = create_mq(mq_name);
    can_msg msg_read;
    TEST_ASSERT_EQUAL(-1, read_mq(mqd, &msg_read));
    close_mq(mqd, mq_name);
}

/** @test */
void test_write_mq_full_queue()
{
    mqd = create_mq(mq_name);
    can_msg msg_to_write = {0};
    for (int i = 0; i < mq_max_messages; i++)
    {
        write_mq(mqd, &msg_to_write);
    }
    TEST_ASSERT_EQUAL(-1, write_mq(mqd, &msg_to_write));
    close_mq(mqd, mq_name);
}

/** @test */
void test_read_and_write_mq_empty_can_msg()
{
    mqd = create_mq(mq_name);
    mqd_t mq_write = open_mq(mq_name);
    can_msg msg_to_write = {0};
    write_mq(mq_write, &msg_to_write);

    can_msg msg_read;
    read_mq(mqd, &msg_read);

    TEST_ASSERT_EQUAL(msg_to_write.identifier, msg_read.identifier);
    close_mq(mqd, mq_name);
}

/** @test */
void test_read_and_write_mq_valid_can_msg()
{
    mqd = create_mq(mq_name);
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
    close_mq(mqd, mq_name);
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_get_mq_attr);
    RUN_TEST(test_create_and_close_mq);
    RUN_TEST(test_create_mq_fail);
    RUN_TEST(test_close_unopened_mq_fail);
    RUN_TEST(test_close_mq_fail_unlink);
    RUN_TEST(test_open_mq);
    RUN_TEST(test_open_mq_fail);
    RUN_TEST(test_read_mq_empty_queue);
    RUN_TEST(test_write_mq_full_queue);
    RUN_TEST(test_read_and_write_mq_empty_can_msg);
    RUN_TEST(test_read_and_write_mq_valid_can_msg);
    return UNITY_END();
}