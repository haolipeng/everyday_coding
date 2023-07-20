#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>

#define FIFO_NAME "/tmp/testp"
#define MAX_SIZE (1024 * 10)
#define BUF_SIZE 1024
int main()
{
    int res = -1;
    int open_mode = O_WRONLY;//block mode until add non block flag
    int pipe_fd = -1;
    char buffer[BUF_SIZE + 1] = "haolipeng";

    //1.access file
    if(access(FIFO_NAME, F_OK) == -1){
        res = mkfifo(FIFO_NAME, 0777);
        if(res != 0){
            fprintf(stderr, "Could not create fifo %s\n", FIFO_NAME);
            return -1;
        }
    }

    //2.open pipe
    int bytes_send = 0;
    pipe_fd = open(FIFO_NAME, open_mode);
    if(pipe_fd != -1){
        while(bytes_send < MAX_SIZE){
	    sleep(3);
            res = write(pipe_fd, buffer, BUF_SIZE);
            if(res == -1){
                fprintf(stderr, "write error in pipe\n");
                continue;
            }
            bytes_send += res;
        }
        close(pipe_fd);
    }else{
        printf("open %s failed\n", FIFO_NAME);
        return -1;
    }
}
