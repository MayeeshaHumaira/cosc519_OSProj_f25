#include <stdio.h>
#include <unistd.h>     // getpid, getppid, sleep
#include <pthread.h>    // pthread_t, pthread_create, pthread_join
#include <stdint.h>     // intptr_t for portable int<->pointer casts

#define N 2                              // use a compile-time constant

void *thread(void *vargp);
const char **ptr;                        // global pointer to array of strings

int main(void)
{
    pthread_t tid[N];                    // thread IDs

    static const char *msgs[N] = {       // sized by #define N (not a VLA)
        "Hello from foo",
        "Hello from bar"
    };

    printf("Parent thread started with PID= %d and parent PID %d\n",
           getpid(), getppid());

    ptr = msgs;                          // let worker threads see the messages

    for (int i = 0; i < N; i++) {
        pthread_create(&tid[i], NULL, thread, (void *)(intptr_t)i);
    }

   // for (int i = 0; i < N; i++) {
   //     pthread_join(tid[i], NULL);
   // }

    return 0;
}

void *thread(void *vargp)
{
    int myid = (int)(intptr_t)vargp;     // recover small integer index portably
    static int cnt = 0;                  // shared across threads (demonstration)

    printf("[%d]: %s (cnt=%d) with PID= %d and parent PID %d\n",
           myid, ptr[myid], ++cnt, getpid(), getppid());

    int i = cnt;
    for (;; i++) {                       // infinite loop
        printf("[%d] %d\n", myid, i);
        sleep(cnt);
    }

    return NULL;
}

