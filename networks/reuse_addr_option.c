#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

#define LOCAL_PORT (12345)

static void test_udp_any_addr_same_port_diff_ifs(void){
    int udp_f1 = socket(AF_INET, SOCK_DGRAM, 0);
    int udp_f2 = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in addr_in;
    memset(&addr_in, 0, sizeof(addr_in));
    addr_in.sin_family = AF_INET;
    addr_in.sin_port = htons(LOCAL_PORT);
    addr_in.sin_addr.s_addr = INADDR_ANY;

    char* opt = "eth0";
    if(setsockopt(udp_f1, SOL_SOCKET, SO_BINDTODEVICE, opt, strlen(opt))){
        printf("UDP1 fail to bind eth0. %s:%d\n", strerror(errno), errno);
    }
}