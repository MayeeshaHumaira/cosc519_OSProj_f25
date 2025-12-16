#include <stdio.h>
#include <sys/types.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
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
sem_t *produced_slots;
sem_t *processed_slots;
sem_t *mutex;

static void Put(char item)
{
    sem_wait(empty_slots);
    sem_wait(mutex);

    s->buffer[s->nextIn] = item;
    s->nextIn = (s->nextIn + 1) % BUFF_SIZE;

    sem_post(mutex);
    printf("Producing %c ... [pid=%d]\n", item, (int)getpid());
    sem_post(produced_slots);                 // item is now available for PROCESSOR
}

static void Producer(void)
{
    for (int i = 0; i < 10; i++) {            
        sleep(rand() % 3);
        Put((char)('A' + (i % 26)));
    }
}

int main(void)
{
    // open/create semaphores
    mutex          = sem_open(sem_name_mutex, O_CREAT, 0644, 1);
    processed_slots= sem_open(sem_name_ready, O_CREAT, 0644, 0);
    produced_slots = sem_open(sem_name_prod,  O_CREAT, 0644, 0);
    empty_slots    = sem_open(sem_name_empty, O_CREAT, 0644, 10);  

    // allocate the shared memory segment
    key_t key = 1234;
    int shmid;
    if ((shmid = shmget(key, sizeof(shared_data), IPC_CREAT | 0666)) < 0) {
        perror("Shmget");
        exit(1);
    }
    if ((shm = (shared_data *)shmat(shmid, NULL, 0)) == (shared_data *)-1) {
        perror("Shmat");
        exit(1);
    }

    s = shm;
    s->nextIn   = 0;     
    // nextProc/nextOut set by the other processes

    Producer();

    // detach
    shmdt((void *)shm);
    return 0;
}

