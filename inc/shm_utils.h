#ifndef SHM_UTILS_H
#define SHM_UTILS_H
#include <semaphore.h>

typedef struct
{
    int rpm;
    int speed;
    int temp;
} sensors_info;

typedef struct
{
    char *shm;
    sem_t *sem;
} shm_args;

int create_shm(char *shm_name);

int open_shm(char *shm_name);

char *map_shm(int shm_fd, int flag);

sem_t *create_sem(char *sem_name);

sem_t *open_sem(char *sem_name);

void read_shm(char *shm_ptr, sem_t *sem, sensors_info *sinfo);

#endif