#include <stdio.h>
#include <mqueue.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include "mq_utils.h"
#include "constants.h"

mqd_t sensors_mq;
pid_t sensors_pid, controller_pid, actuators_pid;

void terminate_execution(int sig)
{
    printf("Closing message queue\n");
    close_mq(sensors_mq, SENSORS_MQ);

    printf("Closing shared memory\n");
    //close_shm();
    exit(0);
}

pid_t create_processes(char *process_name)
{
    pid_t child_pid = fork();

    if(child_pid < 0)
    {
        perror("Error creating auxiliary process \n");
        exit(1);
    }
    else if(child_pid == 0)
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

    //Initialize resources
    sensors_mq = create_mq(SENSORS_MQ);
    //create_shm();

    //Create auxiliary processes
    char *sensors_process = "../bin/sensors";
    char *controller_process = "../bin/controller";
    char *actuators_process = "../bin/actuators";

    sensors_pid = create_processes(sensors_process);
    controller_pid = create_processes(controller_process);
    actuators_pid = create_processes(actuators_process);

    // Read data from file
    while(1)
    {
        
    }

    signal(SIGINT, terminate_execution);

    return EXIT_SUCCESS;
}