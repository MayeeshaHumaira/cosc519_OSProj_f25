#include <stdio.h>
#include <pthread.h>

/* Structure used to pass multiple parameters to a thread */
struct char_print_parms {
    char character;
    int  count;
};

/* Thread start routine */
void *char_print(void *parms)
{
    struct char_print_parms *p = (struct char_print_parms *)parms;

    for (int i = 0; i < p->count; i++) {
        printf("%c", p->character);
    }
    printf("\n");

    return NULL;
}

int main(void)
{
    pthread_t thread1_id;
    pthread_t thread2_id;

    struct char_print_parms thread1_args;
    struct char_print_parms thread2_args;

    /* Create a new thread to print 300 'x's. */
    thread1_args.character = 'x';
    thread1_args.count     = 300;
    pthread_create(&thread1_id, NULL, char_print, &thread1_args);

    /* Create a new thread to print 200 'o's. */
    thread2_args.character = 'o';
    thread2_args.count     = 200;
    pthread_create(&thread2_id, NULL, char_print, &thread2_args);

    /* Wait for both threads to finish */
    pthread_join(thread1_id, NULL);
    pthread_join(thread2_id, NULL);

    return 0;
}

