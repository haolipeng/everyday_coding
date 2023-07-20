//
// Created by root on 22-9-14.
//

#include <cstdio>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sched.h>
#include <dirent.h>

static int enter_netns(const char *netns)
{
    int curfd, netfd;

    if ((curfd = open("/proc/self/ns/net", O_RDONLY)) == -1) {
        //DEBUG_ERROR(DBG_CTRL, "failed to open current network namespace\n");
        return -1;
    }
    if ((netfd = open(netns, O_RDONLY)) == -1) {
        //DEBUG_ERROR(DBG_CTRL, "failed to open network namespace: netns=%s\n", netns);
        close(curfd);
        return -1;
    }
    if (setns(netfd, CLONE_NEWNET) == -1) {
        //DEBUG_ERROR(DBG_CTRL, "failed to enter network namespace: netns=%s error=%s\n", netns, strerror(errno));
        close(netfd);
        close(curfd);
        return -1;
    }
    close(netfd);
    return curfd;
}

static int restore_netns(int fd)
{
    if (setns(fd, CLONE_NEWNET) == -1) {
        //DEBUG_ERROR(DBG_CTRL, "failed to restore network namespace: error=%s\n", strerror(errno));
        close(fd);
        return -1;
    }
    close(fd);
    return 0;
}

bool is_valid_ip(const char* ip){
    if(ip[0] == '1' && ip[1] == '2' && ip[2] == '7'){
        return false;
    }

    struct sockaddr_in sa;
    int result = inet_pton(AF_INET,ip,&(sa.sin_addr));
    return result != 0;
}

//obtain_container_ip only used for obtain container ip
//test assign argument to "/proc/7517/ns/net"
char* obtain_container_ip(const char *netns) {
    const int BUF_SIZE = 16;

    //1.enter network namespace of netns param
    int cur_fd = enter_netns(netns);
    if(-1 == cur_fd){
        printf("enter_netns function failed!\n");
        return NULL;
    }

    const char* commands[] = {
            "ifconfig eth0 | grep \"inet \" | awk '{print $2}'",
            "ip addr show eth0 | grep -Po 'inet \\K[\\d.]+'",
            "ip route | awk '/scope/ {print $9}'",
    };

    //2.execute ip and ifconfig command
    char* buffer = (char *)malloc(sizeof(char) * BUF_SIZE);
    int commandCnt = sizeof(commands)/sizeof(const char*);
    for(int i=0; i < commandCnt; i++)
    {
        FILE* fp;
        memset(buffer,0,BUF_SIZE);
        fp = popen(commands[i],"r");
        if(NULL == fp)
        {
            printf("popen %s failed\n",commands[i]);
            continue;
        }

        fgets(buffer, BUF_SIZE, fp);

        int len = strlen(buffer);

        //case1: buffer is empty
        if(len <= 0){
            pclose(fp);
            continue;
        }

        //case2:the tail of buffer contain '\n'
        if('\n' == buffer[len -1])
        {
            buffer[len -1]  = '\0';
        }

        //case3:ip is invalid ip, 127.0.0.1 or 192.169.abc.10 etc
        if(is_valid_ip(buffer)){
            printf("%s\n",buffer);
            pclose(fp);
            break;
        }

        pclose(fp);
    }

    //restore network namespace
    restore_netns(cur_fd);

    if(strlen(buffer) > 0)
        return buffer;

    return NULL;
}



int main(){
    //for test ,container process id is 7517
    char * container_ip = obtain_container_ip("/proc/7517/ns/net");
    printf("container ip:%s\n",container_ip);
    if(container_ip != NULL){
        free(container_ip);
    }
}


