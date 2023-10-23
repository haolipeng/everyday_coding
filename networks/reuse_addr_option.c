#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

#define LOCAL_PORT (12345)

//测试：udp使用任意地址、相同端口，但是不同的interface网口，能否成功?
static void test_udp_any_addr_same_port_diff_ifs(void){
    int udp_f1 = socket(AF_INET, SOCK_DGRAM, 0);
    int udp_f2 = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in addr_in;
    memset(&addr_in, 0, sizeof(addr_in));
    addr_in.sin_family = AF_INET;
    addr_in.sin_port = htons(LOCAL_PORT);
    addr_in.sin_addr.s_addr = INADDR_ANY;

    char* opt = "ens33";
    if(setsockopt(udp_f1, SOL_SOCKET, SO_BINDTODEVICE, opt, strlen(opt))){
        printf("UDP1 fail to bind eth0. %s:%d\n", strerror(errno), errno);
    }

    opt = "ens34";
    if(setsockopt(udp_f2,SOL_SOCKET, SO_BINDTODEVICE, opt, strlen(opt))){
        printf("UDP2 fail to bind eth1. %s:%d\n", strerror(errno), errno);
    }

    if(bind(udp_f1, (const struct sockaddr *)&addr_in, sizeof(addr_in)) != 0){
        printf("UDP1 fail to bind any addr and port(%d) on eth0\n", LOCAL_PORT);
    }

    if(bind(udp_f2, (const struct sockaddr *)&addr_in, sizeof(addr_in)) != 0){
        printf("UDP2 fail to bind any addr and port(%d) on eth1\n", LOCAL_PORT);
    }

    close(udp_f1);
    close(udp_f2);
}

//测试udp采用不同网络地址，但是相同端口号
static void test_udp_diff_addr_same_port(struct in_addr* addr){
    int udp_f1 = socket(AF_INET, SOCK_DGRAM, 0);
    int udp_f2 = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in addr_in;
    memset(&addr_in, 0, sizeof(addr_in));
    addr_in.sin_family = AF_INET;
    addr_in.sin_port = htons(LOCAL_PORT);
    addr_in.sin_addr = *addr;

    //bind udp_f1
    if(bind(udp_f1, (const struct sockaddr *)&addr_in, sizeof(addr_in)) != 0){
        printf("UDP1 fail to bind any addr and port(%d)\n", LOCAL_PORT);
    }

    //bind udp_f2
    addr_in.sin_addr.s_addr = 0x0100007f;
    if(bind(udp_f2, (const struct sockaddr *)&addr_in, sizeof(addr_in)) != 0){
        printf("UDP2 fail to bind any addr and port(%d)\n", LOCAL_PORT);
    }

    close(udp_f1);
    close(udp_f2);

    printf("UDP could bind two different addr with same port without SO_REUSEADDR\n");
}

static void test_udp_any_addr_same_port(struct in_addr* addr){
    int udp_fd1 = socket(AF_INET, SOCK_DGRAM, 0);
    int udp_fd2 = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in addr_in;
    memset(&addr_in, 0, sizeof(addr_in));
    addr_in.sin_family = AF_INET;
    addr_in.sin_port = htons(LOCAL_PORT);
    addr_in.sin_addr.s_addr = INADDR_ANY;

    int enable = 1;

    //设置端口复用属性
    setsockopt(udp_fd1, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int ));
    setsockopt(udp_fd2, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int ));

    //bind udp socket
    int res = -1;
    res = bind(udp_fd1, (const struct sockaddr*)&addr_in, sizeof(addr_in));
    if(res != 0){
        printf("UDP1 fail to bind port(%d)\n", LOCAL_PORT);
    }

    res = bind(udp_fd2, (const struct sockaddr*)&addr_in, sizeof(addr_in));
    if(res != 0){
        printf("UDP2 fail to bind port(%d)\n", LOCAL_PORT);
    }

    close(udp_fd1);
    close(udp_fd2);

    printf("UDP could bind two local addrs and port with SO_REUSEADDR");
}

static void test_udp_local_and_any_addr_same_port(struct in_addr* addr){
    int udp_fd1 = socket(AF_INET, SOCK_DGRAM, 0);
    int udp_fd2 = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in addr_in;
    memset(&addr_in, 0, sizeof(addr_in));
    addr_in.sin_family = AF_INET;
    addr_in.sin_port = htons(LOCAL_PORT);
    addr_in.sin_addr = *addr;

    int enable = 1;
    setsockopt(udp_fd1, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
    setsockopt(udp_fd2, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

    int res = -1;
    res = bind(udp_fd1, (const struct sockaddr*) &addr_in, sizeof(addr_in));
    if(res != 0){
        printf("UDP1 fail to bind local addr and  port(%d)\n", LOCAL_PORT);
    }
    res = bind(udp_fd2, (const struct sockaddr*) &addr_in, sizeof(addr_in));
    if(res != 0){
        printf("UDP2 fail to bind local addr and  port(%d)\n", LOCAL_PORT);
    }

    //关闭句柄
    close(udp_fd1);
    close(udp_fd2);

    printf("UDP could bind one local addr and one any addr with same port with SO_REUSEADDR\n");
}

//测试udp绑定本地地址相同端口，并采用SO_REUSEADDR
static void test_udp_local_addr_same_port(struct in_addr* addr){
    int udp_fd1 = socket(AF_INET, SOCK_DGRAM, 0);
    int udp_fd2 = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in addr_in;
    memset(&addr_in, 0, sizeof(addr_in));
    addr_in.sin_family = AF_INET;
    addr_in.sin_port = htons(LOCAL_PORT);
    addr_in.sin_addr = *addr;

    int enable = 1;
    //设置端口复用属性
    setsockopt(udp_fd1, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int ));
    setsockopt(udp_fd2, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int ));

    //bind udp socket
    int res = -1;
    res = bind(udp_fd1, (const struct sockaddr*)&addr_in, sizeof(addr_in));
    if(res != 0){
        printf("UDP1 fail to bind port(%d)\n", LOCAL_PORT);
    }

    res = bind(udp_fd2, (const struct sockaddr*)&addr_in, sizeof(addr_in));
    if(res != 0){
        printf("UDP2 fail to bind port(%d)\n", LOCAL_PORT);
    }

    close(udp_fd1);
    close(udp_fd2);

    printf("UDP could bind two local addrs and port with SO_REUSEADDR");
}

//测试tcp和udp能否使用同一个端口
//测试结果：可以
static void test_tcp_udp_same_ports(struct in_addr* addr){
    //create tcp socket
    int tcp_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(-1 == tcp_fd){
        printf("Fail to create tcp socket\n");
        return ;
    }

    //create udp socket
    int udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(-1 == udp_fd){
        printf("Fail to create udp socket\n");
        return ;
    }

    //assign value to addr_in struct
    struct sockaddr_in addr_in;
    memset(&addr_in, 0, sizeof(addr_in));

    addr_in.sin_family = AF_INET;
    addr_in.sin_port = htons(LOCAL_PORT);
    addr_in.sin_addr = *addr;

    //tcp bind function
    int res = -1;
    res = bind(tcp_fd, (const struct sockaddr*)&addr_in, sizeof(addr_in));
    if(res != 0){
        printf("Tcp fail to bind port(%d)\n", LOCAL_PORT);
    }

    //udp bind function
    res = bind(udp_fd, (const struct sockaddr*)&addr_in, sizeof(addr_in));
    if(res != 0){
        printf("UDP fail to bind port(%d)\n", LOCAL_PORT);
    }

    //close fd of tcp and udp
    close(tcp_fd);
    close(udp_fd);

    printf("TCP and UDP could bind same port seperately \n");
}

int main(int argc, const char** argv){
    struct in_addr addr;
    if(argc < 2){
        printf("Please specify local ip addresss\n");
        return -1;
    }

    memset(&addr, 0 , sizeof(addr));
    if(inet_pton(AF_INET, argv[1], &addr) != 1){
        printf("invalid IP address");
        return -1;
    }

    test_tcp_udp_same_ports(&addr);
    test_udp_any_addr_same_port_diff_ifs();

    return 0;
}