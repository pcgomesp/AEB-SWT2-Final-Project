#include <stdio.h>
#include <mqueue.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "mq_utils.h"
#include "constants.h"

mqd_t sensors_mq, actuators_mq;
pid_t sensors_pid, controller_pid, actuators_pid;

void wait_terminate_execution()
{
    waitpid(sensors_pid, NULL, 0);
    waitpid(controller_pid, NULL, 0);
    waitpid(actuators_pid, NULL, 0);

    printf("Closing message queue\n");
    close_mq(sensors_mq, SENSORS_MQ);
    close_mq(actuators_mq, ACTUATORS_MQ);

}

void terminate_execution(int sig)
{
    printf("Closing message queue\n");
    close_mq(sensors_mq, SENSORS_MQ);
    close_mq(sensors_mq, ACTUATORS_MQ);

    printf("Closing child processes\n");
    kill(sensors_pid, SIGTERM);
    kill(controller_pid, SIGTERM);
    kill(actuators_pid, SIGTERM);

    waitpid(sensors_pid, NULL, 0);
    waitpid(controller_pid, NULL, 0);
    waitpid(actuators_pid, NULL, 0);

    printf("Execution terminated\n");

    exit(0);
}

pid_t create_processes(char *process_name)
{
    pid_t child_pid = fork();

    if (child_pid < 0)
    {
        perror("Error creating auxiliary process \n");
        exit(1);
    }
    else if (child_pid == 0)
    {
        if (execl(process_name, process_name, NULL) == -1)
        {
            perror("Error executing the process");
            exit(1);
        }
    }

    return child_pid;
}

int main()
{
    printf("Main process PID: %d\n", getpid());

    // Initialize resources
    sensors_mq = create_mq(SENSORS_MQ);
    sensors_mq = create_mq(ACTUATORS_MQ);

    // Create auxiliary processes
    char *sensors_process = "./bin/sensors_bin";
    char *controller_process = "./bin/aeb_controller_bin";
    char *actuators_process = "./bin/actuators_bin";

    sensors_pid = create_processes(sensors_process);
    controller_pid = create_processes(controller_process);
    actuators_pid = create_processes(actuators_process);

    signal(SIGINT, terminate_execution);

    wait_terminate_execution();

    printf("Execution finished, check out log/log.txt for info!\n");

    return EXIT_SUCCESS;
}