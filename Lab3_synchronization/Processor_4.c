#include <stdio.h>
#include <sys/types.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <ctype.h>

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

static void ProcessOne(void)
{
    sem_wait(produced_slots);      // wait for item produced
    sem_wait(mutex);

    char in = s->buffer[s->nextProc];
    char out = (char)tolower((unsigned char)in);  // convert case
    s->buffer[s->nextProc] = out;                 // write back in place
    s->nextProc = (s->nextProc + 1) % BUFF_SIZE;

    sem_post(mutex);
    printf("Processing %c -> %c ... [pid=%d]\n", in, out, (int)getpid());
    sem_post(processed_slots);     // now ready for consumer
}

static void Processor(void)
{
    for (int i = 0; i < 10; i++) { // process same number as produced/consumed
        sleep(rand() % 3);
        ProcessOne();
    }
}

int main(void)
{
    // open existing semaphores (they are created by Producer)
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
    s->nextProc = 0;

    Processor();

    shmdt((void *)shm);
    return 0;
}

