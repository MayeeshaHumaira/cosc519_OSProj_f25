#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#define BUFF_SIZE 20

char buffer[BUFF_SIZE];
int nextIn = 0;
int nextOut = 0;

void Put(char item)
{
    buffer[nextIn] = item;
    nextIn = (nextIn + 1) % BUFF_SIZE;
    printf("Producing %c ...\n", item);
}
void * Producer()
{
    int i;
    for(i = 0; i < 15; i++)
    {
        sleep(rand()%9);
        Put((char)('A'+ i % 26));
    }
}

void Get(char item)
{
    item = buffer[nextOut];
    nextOut = (nextOut + 1) % BUFF_SIZE;

    printf("Consuming %c ...\n", item);
}
void * Consumer()
{
    int i;
    char  item;
    for(i = 0; i < 15; i++)
    {
        sleep(rand()%4);
        Get(item);
    }
}
void main()
{
        pthread_t pid, cid;
        pthread_create(&pid, NULL, Producer, NULL);
        pthread_create(&cid, NULL, Consumer, NULL);
        pthread_join(pid, NULL);
        pthread_join(cid, NULL);
}

