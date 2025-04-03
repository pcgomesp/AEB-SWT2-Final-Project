#include <sys/stat.h>
#include <sys/wait.h>
#include <mqueue.h>
#include "mq_utils.h"
#include "unity.h"
#include "constants.h"
#include <setjmp.h>

// Declare the global variables
extern mqd_t sensors_mq, actuators_mq;
extern pid_t sensors_pid, controller_pid, actuators_pid;

// Declare the functions from main.c
void wait_terminate_execution();
void terminate_execution();

jmp_buf test_exit_buf;

void __wrap_exit(int status)
{
    printf("Mocked exit() called with status: %d\n", status);
    longjmp(test_exit_buf, 1);
}

int __wrap_waitpid(pid_t pid, int *status, int options)
{
    return pid;
}

int __wrap_kill(pid_t pid, int sig)
{
    printf("Mocked kill() called with pid: %d, sig: %d\n", pid, sig);
    return 0;
}

void __wrap_close_mq(mqd_t mq, const char *name)
{
    printf("Mocked close_mq() called for queue: %s\n", name);
}

mqd_t __wrap_create_mq(const char *name)
{
    printf("Mocked create_mq() called for queue: %s\n", name);
    return (mqd_t) 1; // Return a non-zero value to simulate a valid message queue descriptor.
}

void setUp()
{
    // set stuff up here
}

void tearDown()
{
    // clean stuff up here
}

void test_wait_terminate_execution()
{
    if (setjmp(test_exit_buf) == 0)
    {
        wait_terminate_execution();
        TEST_FAIL_MESSAGE("Execution should have stopped after terminate_execution()");
    }

    struct stat buffer_sensors;
    int exists_sensors = stat("/dev/mqueue/mq_aeb_sensors", &buffer_sensors);

    struct stat buffer_actuators;
    int exists_actuators = stat("/dev/mqueue/mq_aeb_actuators", &buffer_actuators);

    exists_sensors = stat("/dev/mqueue/mq_aeb_sensors", &buffer_sensors);
    exists_actuators = stat("/dev/mqueue/mq_aeb_actuators", &buffer_actuators);
    TEST_ASSERT_EQUAL(-1, exists_sensors);
    TEST_ASSERT_EQUAL(-1, exists_actuators);
}

void test_terminate_execution()
{
    if (setjmp(test_exit_buf) == 0)
    {
        terminate_execution();
        TEST_FAIL_MESSAGE("Execution should have stopped after terminate_execution()");
    }

    struct stat buffer_sensors;
    int exists_sensors = stat("/dev/mqueue/mq_aeb_sensors", &buffer_sensors);

    struct stat buffer_actuators;
    int exists_actuators = stat("/dev/mqueue/mq_aeb_actuators", &buffer_actuators);

    exists_sensors = stat("/dev/mqueue/mq_aeb_sensors", &buffer_sensors);
    exists_actuators = stat("/dev/mqueue/mq_aeb_actuators", &buffer_actuators);
    TEST_ASSERT_EQUAL(-1, exists_sensors);
    TEST_ASSERT_EQUAL(-1, exists_actuators);
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_wait_terminate_execution);
    RUN_TEST(test_terminate_execution);
    return UNITY_END();
}