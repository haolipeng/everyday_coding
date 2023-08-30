#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void){
    int fd = open("./test_flock.txt", O_RDWR | O_CREAT, 0644);
    if(-1 == fd){
        printf("Fail to open file\n");
        exit(1);
    }

    int ret = flock(fd, LOCK_EX);
    if(-1 == ret){
        printf("Fail to flock\n");
        exit(1);
    }
    printf("Parent get the flock at the first time\n");

    pid_t child = fork();
    if(-1 == child){
        printf("Fail to fork\n");
        exit(1);
    }else if(0 == child){
        printf("Child sleeps now\n");
        sleep(30);
        printf("Child wakes up and exits\n");
        exit(1);
    }

    flock(fd, LOCK_UN);
    printf("Parent release the lock\n");
    printf("Parent tries to get flock again\n");
    flock(fd, LOCK_EX);

    waitpid(child, NULL, 0);
    printf("Parent exit\n");
    return 0;
}
