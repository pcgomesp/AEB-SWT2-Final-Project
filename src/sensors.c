#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "constants.h"
#include "mq_utils.h"
#include "sensors_input.h"
#include <pthread.h>
#include <stdbool.h>

void* getSensorsData(void *arg);

pthread_t sensors_id;
char *shm_ptr;
sensors_input_data sensorsData = {
    .vehicle_velocity = 0.0, 
    .has_obstacle = false, 
    .obstacle_distance = 0.0, 
    .brake_pedal = false, 
    .accelerator_pedal = false, 
    .on_off_aeb_system = true
};

int main(){
    int sensors_thr;
    // Falta o SHM_LOCATION
    // int shm_fd = shm_open(SHM_LOCATION, O_RDWR , 0666); // Acess to shared memory, not yet implemented
    // shm_ptr = (char *)mmap(0, BUFFER_SIZE_SHM, PROT_WRITE, MAP_SHARED, shm_fd, 0); // Shared mem mapping to program internal mem
    
    // FALTA O SMPH_NAME
    // smph = sem_open(SMPH_NAME, 0); // Acess to Posix Semaphore create on main.c
    // if (smph == SEM_FAILED) {
    //     perror("Sensors: it wasn't possible to acess the semaphore");
    //     exit(51);
    // }

    mqd_t mq_sender = open_mq(SENSORS_MQ);

    // int speed = 0; // Apagar isso daqui
    // while(1)
    // {
    //     speed += rand() % 20 - 10;
    //     char buffer[MQ_MAX_MSG_SIZE];
    //     sprintf(buffer, "S: %d", speed);
    //     printf("%s\n",buffer);

    //     write_mq(mq_sender, buffer);
    //     sleep(1);
    // }

    sensors_thr = pthread_create(&sensors_id, NULL, getSensorsData, NULL);
    if(sensors_thr != 0){
        perror("Sensors: it wasn't possible to create the associated thread\n");
        exit(52);
    }
    sensors_thr = pthread_join(sensors_id, NULL);
    //close(shm_fd);

    return 0;
}

void* getSensorsData(void *arg){
    // while(1){
    //     // MUST HAVE a Semaphore here

    //     // Take data from the shared memory to sensorsData struct variable
    //     // Convert to can_msg format
    //     // Pass all the data, writing 4 different can_msg messages 
        
    //     sleep(1);
    // }

    printf("Placeholder\n");
}