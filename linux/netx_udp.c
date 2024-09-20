#include "netx_udp.h"

#ifdef __linux__ // only support linux

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define NETX_UDP_MTU (1500)

typedef struct
{
    int sockfd;
    struct sockaddr_in peer_addr;
    struct sockaddr_in local_addr;

    socklen_t addrlen;

    pthread_t recv_thread;
    int recv_thread_started;

    void *priv;
} udp_t;

static int parse_address(const char *address_str, struct sockaddr_in *addr);
static void udp_recv_thread_start(udp_t *dev);
static void udp_recv_thread_stop(udp_t *dev);

static int32_t _udp_netx_send(netx *self, const uint8_t *data, uint32_t len)
{
    int32_t ret = 0;
    udp_t *dev = (udp_t *)self->priv;
    if (dev)
    {
        ret = sendto(dev->sockfd, data, len, 0, (struct sockaddr *)&dev->peer_addr, dev->addrlen);
    }
    return ret;
}

static int32_t _udp_netx_ctrl(netx *self, uint32_t cmd, void *data, uint32_t len)
{
    int32_t ret = 0;
    udp_t *dev = (udp_t *)self->priv;

    if (cmd == UDP_CTRL_CMD_BIND)
    {
        if (parse_address((const char *)data, &dev->local_addr) != 0)
        {
            ret = -1;
        }
        else
        {
            if (bind(dev->sockfd, (struct sockaddr *)&dev->local_addr, dev->addrlen) < 0)
            {
                perror("bind failed");
                ret = -1;
            }
        }
    }

    return ret;
}

static int32_t _udp_netx_get_mtu(netx *self)
{
    return NETX_UDP_MTU;
}

static int32_t _udp_netx_start(netx *self, netx_on_data on_data)
{
    int32_t ret = 0;
    long sockfd;
    struct sockaddr_in addr;
    udp_t *dev = (udp_t *)self->priv;

    if (on_data)
    {
        udp_recv_thread_start(dev);
    }

    return ret;
}

int32_t _udp_netx_stop(netx *self)
{
    int32_t ret = 0;

    udp_t *dev = (udp_t *)self->priv;
    if (dev)
    {
        udp_recv_thread_stop(dev);
    }

    return ret;
}

static netx_interface_t g_udp_netx_interface = {
    .send = _udp_netx_send,

    .get_mtu = _udp_netx_get_mtu,

    .ctrl = _udp_netx_ctrl,

    .start = _udp_netx_start,
    .stop = _udp_netx_stop,
};

int32_t NetxUdpCreate(netx *self, const char *desc)
{
    int32_t ret = -1;
    long sockfd;
    udp_t *dev = NULL;
    if (!self)
    {
        return -1;
    }

    dev = (udp_t *)malloc(sizeof(udp_t));
    if (!dev)
    {
        return -1;
    }

    memset(dev, 0, sizeof(udp_t));

    // 创建 UDP 套接字
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    dev->sockfd = sockfd;
    printf("socket created\n");

    if (parse_address(desc, &dev->peer_addr) != 0)
    {
        free(dev);
        return -1;
    }
    dev->addrlen = sizeof(dev->peer_addr);

    dev->priv = self;

    self->interface = &g_udp_netx_interface;
    self->priv = dev;
    ret = 0;

    return ret;
}

int32_t NetxUdpDestory(netx *self)
{
    int32_t ret = -1;

    if (!self)
    {
        return -1;
    }

    if (self->state == NETX_START)
    {
        self->interface->stop(self);
    }

    free(self->priv);

    self->interface = NULL;
    self->priv = NULL;
    ret = 0;

    return ret;
}

static int parse_address(const char *address_str, struct sockaddr_in *addr)
{
    int port;
    char ip[INET_ADDRSTRLEN];
    char *port_str;

    // 分离IP地址和端口号
    port_str = strchr(address_str, ':');
    if (port_str == NULL)
    {
        fprintf(stderr, "Invalid address format.\n");
        return -1;
    }

    // 复制IP地址部分
    size_t ip_len = port_str - address_str;
    if (ip_len >= INET_ADDRSTRLEN)
    {
        fprintf(stderr, "IP address too long.\n");
        return -1;
    }
    strncpy(ip, address_str, ip_len);
    ip[ip_len] = '\0'; // 确保字符串以空字符结尾

    // 解析端口号
    port_str++; // 跳过冒号
    port = atoi(port_str);
    if (port <= 0 || port > 65535)
    {
        fprintf(stderr, "Invalid port number.\n");
        return -1;
    }

    printf("ip: %s, port: %d\n", ip, port);

    // 初始化sockaddr_in结构体
    memset(addr, 0, sizeof(struct sockaddr_in));
    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = inet_addr(ip);
    addr->sin_port = htons(port);

    return 0;
}

static void *udp_recv_thread(void *arg)
{
    udp_t *dev = (udp_t *)arg;
    int sockfd = dev->sockfd;
    uint8_t buf[NETX_UDP_MTU] = {0};

    printf("udp_recv_thread started\n");

    while (1)
    {
        memset(buf, 0, sizeof(buf));
        int len = recvfrom(sockfd, buf, sizeof(buf), 0, NULL, 0);
        if (len > 0)
        {
            NetxOnData(dev->priv, buf, len);
        }
    }
}

static void udp_recv_thread_start(udp_t *dev)
{
    if (!dev->recv_thread_started)
    {
        pthread_create(&dev->recv_thread, NULL, udp_recv_thread, (void *)dev);
        dev->recv_thread_started = 1;
    }
}

static void udp_recv_thread_stop(udp_t *dev)
{
    if (dev->recv_thread_started)
    {
        pthread_cancel(dev->recv_thread);
        dev->recv_thread_started = 0;
    }
}

#endif
