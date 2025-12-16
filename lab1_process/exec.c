#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

int main(void) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        exit(1);
    } else if (pid == 0) {
        // Child process
        printf("\nChild: PID = %d\n", getpid());
    } else {
        // Parent process
        printf("Parent: PID = %d\n", getpid());
    }

    return 0;
}

