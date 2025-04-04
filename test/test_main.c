#include <sys/stat.h>
#include <sys/wait.h>
#include <mqueue.h>
#include <setjmp.h>
#include <string.h>
#include "mq_utils.h"
#include "unity.h"
#include "constants.h"

// Declare the global variables
extern mqd_t sensors_mq, actuators_mq;
extern pid_t sensors_pid, controller_pid, actuators_pid;

// Declare the functions from main.c
void wait_terminate_execution();
void terminate_execution();
pid_t create_processes(char *process_name);

int fork_mock = 0;
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
    return (mqd_t)1; // Return a non-zero value to simulate a valid message queue descriptor.
}

pid_t __wrap_fork()
{
    if (fork_mock == 1)
    {
        printf("Mocked fork() returning child PID 1234\n");
        return 1234;
    }
    else if (fork_mock == 2)
    {
        printf("Mocked fork() returning -1 (failure)\n");
        return -1;
    }
    else
    {
        printf("Mocked fork() returning 0 (child process)\n");
        return 0;
    }
}

int __wrap_execl(const char *path, const char *arg, ...)
{
    printf("Mocked execl() called with path: %s\n", path);

    if (strcmp(path, "/invalid/path") == 0)
    {
        printf("Mocked execl() failing\n");
        return -1;
    }

    return 0;
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

void test_create_processes_fork_success()
{
    printf("\nRunning test: create_processes() when fork succeeds\n");
    fork_mock = 1; // Simulate a successful fork
    pid_t pid = create_processes("mock_process");
    TEST_ASSERT_EQUAL(1234, pid);
}

void test_create_processes_fork_failure()
{
    printf("\nRunning test: create_processes() when fork fails\n");

    if (setjmp(test_exit_buf) == 0) 
    {
        fork_mock = 2; // Simulate a failed fork
        create_processes("mock_process");

        // If we reach here, exit(1) was NOT called (unexpected)
        TEST_FAIL_MESSAGE("create_processes() did not exit as expected");
    } 
    else 
    {
        // If we jump back here, it means exit(1) was called as expected
        printf("Caught exit(1) as expected\n");
    }
}

void test_create_processes_execl_succeeds()
{
    printf("\nRunning test: create_processes() when execl fails\n");

    if (setjmp(test_exit_buf) == 0) 
    {
        fork_mock = 0; // Simulate child process
        create_processes("/valid/path");

        // If we reach here, exit(1) was NOT called (expected)
        TEST_PASS();
    }
}

void test_create_processes_execl_failure()
{
    printf("\nRunning test: create_processes() when execl fails\n");

    if (setjmp(test_exit_buf) == 0) 
    {
        fork_mock = 0; // Simulate child process
        create_processes("/invalid/path");

        // If we reach here, exit(1) was NOT called (unexpected)
        TEST_FAIL_MESSAGE("create_processes() did not exit as expected");
    } 
    else 
    {
        // If we jump back here, it means exit(1) was called as expected
        printf("Caught exit(1) as expected\n");
    }
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_wait_terminate_execution);
    RUN_TEST(test_terminate_execution);
    RUN_TEST(test_create_processes_fork_success);
    RUN_TEST(test_create_processes_fork_failure);
    RUN_TEST(test_create_processes_execl_succeeds);
    RUN_TEST(test_create_processes_execl_failure);
    return UNITY_END();
}