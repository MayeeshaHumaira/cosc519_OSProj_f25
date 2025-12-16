#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

int main()
{
    int pid;
    pid = fork();
    if (pid == -1)
    {
        perror("ERROR - fork \n");
        exit(0);
    }
    else if (pid == 0)
    {
        printf("Child process with pid = %d, and Parent pid = %d\n",
               getpid(), getppid());
        sleep(5);
        printf("After sleeping.  Child process with pid = %d, and Parent pid = %d\n",
               getpid(), getppid());
        exit(0);
    }
    else
    {
        printf("Parent process with pid = %d, and Parent pid = %d\n",
               getpid(), getppid());
        printf("Parent exiting now\n");
        exit(0);
    }
    return 0;
}

