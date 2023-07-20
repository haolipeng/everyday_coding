
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>

#define FIFO_NAME "/tmp/testp"
#define MAX_SIZE (1024 * 1024 * 10)
#define BUF_SIZE 1024
int main()
{
    int res = -1;
    int open_mode = O_RDONLY | O_NONBLOCK;//block mode until add non block flag
    int pipe_fd = -1;
    char buffer[BUF_SIZE + 1] = "haolipeng";

    //1.open pipe
    int bytes_read = 0;
    time_t beginTime,endTime;
    pipe_fd = open(FIFO_NAME, open_mode);
    if(pipe_fd != -1){
        do{
	    beginTime = time(NULL);
            res = read(pipe_fd, buffer, BUF_SIZE);
            bytes_read += res;
	    endTime = time(NULL);
	    printf("read operation cost time:%lds\n", endTime - beginTime);
        } while (res > 0);
        close(pipe_fd);
    }else{
        printf("open %s failed\n", FIFO_NAME);
        return -1;
    }
    printf("Process %d finished read %d bytes\n", getpid(), bytes_read);
}
