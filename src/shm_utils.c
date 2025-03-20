#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "shm_utils.h"
#include "constants.h"

int create_shm(char *shm_name)
{
    int shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, SHM_PERMISSIONS);
    ftruncate(shm_fd, 4096);
    return shm_fd;
}

int open_shm(char *shm_name)
{
    int shm_fd = shm_open(shm_name, O_RDWR, SHM_PERMISSIONS);
    if (shm_fd == -1)
    {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }
    return shm_fd;
}

char *map_shm(int shm_fd, int flag)
{
    char *ptr = (char *)mmap(0, 4096, flag, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED)
    {
        perror("Error mapping shared memory object");
        close(shm_fd);
        exit(EXIT_FAILURE);
    }
    return ptr;
}

sem_t *create_sem(char *sem_name)
{
    sem_t *sem;
    sem = sem_open(sem_name, O_CREAT, SHM_PERMISSIONS, 1);
    if (sem == SEM_FAILED)
    {
        perror("Error creating semaphore");
        exit(EXIT_FAILURE);
    }
    return sem;
}

sem_t *open_sem(char *sem_name)
{
    sem_t *sem;
    sem = sem_open(sem_name, 0);
    if (sem == SEM_FAILED)
    {
        perror("Error opening semaphore");
        exit(EXIT_FAILURE);
    }
    return sem;
}

void read_shm(char *shm_ptr, sem_t *sem, sensors_info *sinfo)
{
    sem_wait(sem);
    switch (shm_ptr[0])
    {
    case 'S':
        sinfo->speed = atoi(shm_ptr + 3);
        break;
    case 'T':
        sinfo->time = atoi(shm_ptr + 3);
        break;
    case 'D':
        sinfo->obs_distance = atoi(shm_ptr + 3);
        break;
    case 'O':
        sinfo->obs_type = atoi(shm_ptr + 3);
        break;
    case 'A':
        sinfo->aeb_status = atoi(shm_ptr + 3);
        break;
    default:
        fprintf(stderr, "Unknown sensor type: %c\n", shm_ptr[0]);
        break;
    }
    sem_post(sem);
}