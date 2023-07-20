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

static int enter_mntns(const char *mntns)
{
    int curfd, mntfd;

    if ((curfd = open("/proc/self/ns/mnt", O_RDONLY)) == -1) {
        //DEBUG_ERROR(DBG_CTRL, "failed to open current network namespace\n");
        return -1;
    }
    if ((mntfd = open(mntns, O_RDONLY)) == -1) {
        //DEBUG_ERROR(DBG_CTRL, "failed to open network namespace: netns=%s\n", netns);
        close(curfd);
        return -1;
    }
    if (setns(mntfd, CLONE_NEWNS) == -1) {
        //DEBUG_ERROR(DBG_CTRL, "failed to enter network namespace: netns=%s error=%s\n", netns, strerror(errno));
        close(mntfd);
        close(curfd);
        return -1;
    }
    close(mntfd);
    return curfd;
}

static int restore_mntns(int fd)
{
    if (setns(fd, CLONE_NEWNS) == -1) {
        //DEBUG_ERROR(DBG_CTRL, "failed to restore network namespace: error=%s\n", strerror(errno));
        close(fd);
        return -1;
    }
    close(fd);
    return 0;
}

int read_iflink_or_ifindex_file(const char* path){
    FILE* fp;
    char buffer[256];

    fp = fopen(path,"r");
    if(NULL == fp){
        printf("fopen %s failed\n",path);
        return -1;
    }

    fgets(buffer, sizeof(buffer), fp);

    int len = strlen(buffer);
    if('\n' == buffer[len -1])
    {
        buffer[len -1]  = '\0';
    }

    fclose(fp);

    int val = atoi(buffer);
    return val;
}

int deal_eth0_ifindex(const char* buffer,int len){
    int result = -1;
    int index = strlen("eth0");

    char strVal[8] = {0};
    int j = 0;
    bool start,end = false;
    while(index < len)
    {
        if(buffer[index] == '@')
        {
            start = true;
        }else if (buffer[index] == ':')
        {
            end = true;
        }
        //between 0 and 9 digit
        if(start && !end && buffer[index] <= '9' && buffer[index] >= '0'){
            strVal[j] = buffer[index];
            j++;
        }

        index++;
    }

    result = atoi(strVal);
    return result;
}

static int _obtain_veth_with_netns(const char* netns){
    const int BUF_SIZE = 64;
    int result = -1;

    if(NULL == netns || 0 == strlen(netns)){
        return result;
    }

    //1.enter network namespace of netns param
    int cur_fd = enter_netns(netns);
    if(-1 == cur_fd){
        printf("enter_netns function failed!\n");
        return result;
    }

    const char* commands[] = {
            "ip link show eth0 | grep eth0 | awk '{print $2}'",
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

        //buffer is such as "eth0@if10:"
        result = deal_eth0_ifindex(buffer,len);

        pclose(fp);
    }

    //restore network namespace
    restore_netns(cur_fd);

    return result;
}

char* obtain_container_veth(const char* mntns,const char* netns){
    //1.enter mount namespace of mntns param
    const int BUF_SIZE = 256;
    char ifindex_path[BUF_SIZE];
    char* nic_name = NULL;

    const char* path = "/sys/class/net";
    DIR *d = NULL;
    struct dirent* dp = NULL;
    int ifindex = -1;
    int iflink = -1;

    iflink = _obtain_veth_with_netns(netns);
    if(-1 == iflink)
    {
        printf("_obtain_veth_with_netns function failed\n");
        int cur_fd = enter_mntns(mntns);
        if(-1 == cur_fd){
            //change to use network space get veth
            printf("enter_mntns function failed\n");
            return NULL;
        }else{
            iflink = read_iflink_or_ifindex_file("/sys/class/net/eth0/iflink");

            if(iflink <= 0)
            {
                printf("read_iflink_or_ifindex_file function failed\n");
                return NULL;
            }

            //3.restore mount namespace
            restore_mntns(cur_fd);
        }
    }

    //4.find ifindex in host equal to iflink
    if(!(d = opendir(path))){
        printf("opendir %s failed\n",path);
        return NULL;
    }

    nic_name = (char*) malloc(sizeof(char) * BUF_SIZE);
    memset(nic_name,0,BUF_SIZE);

    while((dp = readdir(d)) != NULL){
        if( (!strncmp(dp->d_name, ".", 1)) || (!strncmp(dp->d_name, "..", 2)) ){
            continue;
        }

        //generate the path
        sprintf(ifindex_path,"/sys/class/net/%s/ifindex",dp->d_name);
        ifindex = read_iflink_or_ifindex_file(ifindex_path);

        if(iflink == ifindex)
        {
            strncpy(nic_name,dp->d_name,BUF_SIZE);
            break;
        }
    }

    //close dir object
    closedir(d);

    return nic_name;
}

int main(){
    //for test
    char* veth = obtain_container_veth("/proc/6242/ns/mnt","/proc/6242/ns/net");
    printf("veth:%s\n",veth);
    if(veth != NULL)
        free(veth);//don't forget
    return 0;
}