#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/types.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/shm.h>

#define BUFF_SIZE 20

typedef struct {
    char buffer[BUFF_SIZE];
    int  nextIn;
    int  nextProc;
    int  nextOut;
} shared_data;

shared_data *shm, *s;

char sem_name_mutex[]   = "mutex";
char sem_name_empty[]   = "empty_slots";
char sem_name_prod[]    = "produced_slots";
char sem_name_ready[]   = "processed_slots";

sem_t *empty_slots;
sem_t *produced_slots;          // not used here but opened for completeness
sem_t *processed_slots;
sem_t *mutex;

static void Get(char item)
{
    sem_wait(processed_slots);          // wait until item has been processed
    sem_wait(mutex);

    item = s->buffer[s->nextOut];
    s->nextOut = (s->nextOut + 1) % BUFF_SIZE;

    sem_post(mutex);
    printf("Consuming %c ... [pid=%d]\n", item, (int)getpid());
    sem_post(empty_slots);              // slot becomes free again
}

static void Consumer(void)
{
    char item;
    for (int i = 0; i < 10; i++) {
        sleep(rand() % 9);
        Get(item);
    }
}

int main(void)
{
    mutex           = sem_open(sem_name_mutex, O_CREAT, 0644, 1);
    processed_slots = sem_open(sem_name_ready, O_CREAT, 0644, 0);
    produced_slots  = sem_open(sem_name_prod,  O_CREAT, 0644, 0);
    empty_slots     = sem_open(sem_name_empty, O_CREAT, 0644, 10);

    // attach to shared memory created by Producer
    key_t key = 1234;
    int shmid;
    if ((shmid = shmget(key, sizeof(shared_data), 0666)) < 0) {
        perror("Shmget");
        exit(1);
    }
    if ((shm = (shared_data *)shmat(shmid, NULL, 0)) == (shared_data *)-1) {
        perror("Shmat");
        exit(1);
    }

    s = shm;
    s->nextOut = 0;

    Consumer();

    shmdt((void *)shm);
    return 0;
}

