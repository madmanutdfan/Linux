#include <semaphore.h>

#define SemaphoreHost "/semaphore_host"
#define SemaphoreRW "/semaphore_rw"
#define SemaphoreGO "/semaphore_gameover"

sem_t* sem_host;
sem_t* sem_rw;
sem_t* sem_gameover;