#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define BUFF_SIZE 20          // size of the circular buffer
#define NUM_PROD  2           // number of producer threads
#define NUM_CONS  2           // number of consumer threads
#define NITEMS_P  15          // items produced by each producer
#define NITEMS_C  15          // items consumed by each consumer (match totals)

static char buffer[BUFF_SIZE];
static int  nextIn  = 0;
static int  nextOut = 0;

static sem_t empty_slots;      // counts free slots in buffer
static sem_t full_slots;       // counts filled slots in buffer
static sem_t mutex;            // mutual exclusion on buffer

// Put one item into the buffer (blocks if full)
static void Put(char item)
{
    sem_wait(&empty_slots);            // wait for at least one empty slot
    sem_wait(&mutex);                  // enter critical section

    buffer[nextIn] = item;
    nextIn = (nextIn + 1) % BUFF_SIZE;

    sem_post(&mutex);                  // leave critical section
    sem_post(&full_slots);             // we just made one slot full

    printf("Producing %c ... [tid=%lu]\n", item, (unsigned long)pthread_self());
    fflush(stdout);
}

static void *Producer(void *arg)
{
    long id = (long)arg;
    (void)id;                          
    for (int i = 0; i < NITEMS_P; i++) {
        sleep(rand() % 6);             // random delay
        char item = (char)('A' + i % 26);
        Put(item);
    }
    return NULL;
}

// Get one item from the buffer (blocks if empty)
static char Get(void)
{
    sem_wait(&full_slots);             // wait for at least one full slot
    sem_wait(&mutex);                  // enter critical section

    char item = buffer[nextOut];
    nextOut = (nextOut + 1) % BUFF_SIZE;

    sem_post(&mutex);                  // leave critical section
    sem_post(&empty_slots);            // just freed one slot

    printf("Consuming %c ... [tid=%lu]\n", item, (unsigned long)pthread_self());
    fflush(stdout);
    return item;
}

static void *Consumer(void *arg)
{
    (void)arg;
    for (int i = 0; i < NITEMS_C; i++) {
        sleep(rand() % 6);             // random delay
        (void)Get();
    }
    return NULL;
}

int main(void)
{
    srand((unsigned)time(NULL));

    // initialize semaphores
    sem_init(&mutex,       0, 1);
    sem_init(&empty_slots, 0, BUFF_SIZE);
    sem_init(&full_slots,  0, 0);

    // start producers and consumers
    pthread_t prod[NUM_PROD], cons[NUM_CONS];
    for (long i = 0; i < NUM_PROD; i++)
        pthread_create(&prod[i], NULL, Producer, (void*)i);
    for (long i = 0; i < NUM_CONS; i++)
        pthread_create(&cons[i], NULL, Consumer, (void*)i);

    // wait for all threads
    for (int i = 0; i < NUM_PROD; i++) pthread_join(prod[i], NULL);
    for (int i = 0; i < NUM_CONS; i++) pthread_join(cons[i], NULL);

    // cleanup
    sem_destroy(&mutex);
    sem_destroy(&empty_slots);
    sem_destroy(&full_slots);

    return 0;
}

