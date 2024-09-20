

#include "netx_tcp.h"

#ifdef __linux__ // only support linux

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "common.h"

#define NETX_TCP_MTU (1500)

typedef struct
{
    int sockfd;
    struct sockaddr_in remote_addr;
    socklen_t addrlen;

    pthread_t recv_thread;
    int recv_thread_started;

    void *priv;
} tcp_t;

static void _tcp_recv_thread_start(tcp_t *dev);
static void _tcp_recv_thread_stop(tcp_t *dev);

static int32_t _tcp_netx_start(netx *self, netx_on_data on_data)
{
    int32_t ret = 0;
    long sockfd;
    struct sockaddr_in addr;
    tcp_t *dev = (tcp_t *)self->priv;

    // connect
    if (connect(dev->sockfd, (struct sockaddr *)&dev->remote_addr, dev->addrlen) < 0)
    {
        perror("connect failed");
        return -1;
    }

    if (on_data)
    {
        _tcp_recv_thread_start(dev);
    }

    return ret;
}

static int32_t _tcp_netx_stop(netx *self)
{
    int32_t ret = 0;

    tcp_t *dev = (tcp_t *)self->priv;
    if (dev)
    {
        _tcp_recv_thread_stop(dev);

        close(dev->sockfd);
    }

    return ret;
}

static int32_t _tcp_netx_get_mtu(netx *self)
{
    return NETX_TCP_MTU;
}

static int32_t _tcp_netx_send(netx *self, const uint8_t *data, uint32_t len)
{
    int32_t ret = 0;
    tcp_t *dev = (tcp_t *)self->priv;
    if (dev)
    {
        ret = send(dev->sockfd, data, len, 0);
    }
    return ret;
}

static int32_t _tcp_netx_ctrl(netx *self, uint32_t cmd, void *data, uint32_t len)
{
    int32_t ret = 0;
    tcp_t *dev = (tcp_t *)self->priv;
    struct sockaddr_in addr = {0};

    if (cmd == TCP_CTRL_CMD_BIND)
    {
        if (parse_address((const char *)data, &addr) != 0)
        {
            ret = -1;
        }
        else
        {
            if (bind(dev->sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
            {
                perror("bind failed");
                ret = -1;
            }
        }
    }

    return ret;
}

static netx_interface_t g_tcp_netx_interface = {
    .send = _tcp_netx_send,

    .get_mtu = _tcp_netx_get_mtu,

    .ctrl = _tcp_netx_ctrl,

    .start = _tcp_netx_start,
    .stop = _tcp_netx_stop,
};

int32_t NetxTcpCreate(netx *self, const char *desc)
{
    int32_t ret = 0;
    long sockfd;
    tcp_t *dev = NULL;
    if (!self)
    {
        return -1;
    }

    dev = (tcp_t *)malloc(sizeof(tcp_t));
    if (!dev)
    {
        return -1;
    }
    memset(dev, 0, sizeof(tcp_t));

    // 创建 TCP 套接字
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("socket creation failed");
        free(dev);
        return -1;
    }
    dev->sockfd = sockfd;
    printf("socket created\n");

    // 解析地址
    if (parse_address(desc, &dev->remote_addr) != 0)
    {
        free(dev);
        return -1;
    }
    dev->addrlen = sizeof(dev->remote_addr);
    dev->priv = self;

    self->priv = dev;
    self->interface = &g_tcp_netx_interface;

    return ret;
}

int32_t NetxTcpDestory(netx *self)
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

static void *tcp_recv_thread(void *arg)
{
    int32_t ret = 0;
    tcp_t *dev = (tcp_t *)arg;
    int sockfd = dev->sockfd;
    uint8_t buf[NETX_TCP_MTU] = {0};

    printf("tcp_recv_thread started\n");

    while (1)
    {
        memset(buf, 0, sizeof(buf));
        int len = recv(sockfd, buf, sizeof(buf), 0);
        if (len > 0)
        {
            NetxOnData(dev->priv, buf, len, NULL, 0);
        }
    }
}

static void _tcp_recv_thread_start(tcp_t *dev)
{
    if (!dev->recv_thread_started)
    {
        pthread_create(&dev->recv_thread, NULL, tcp_recv_thread, (void *)dev);
        dev->recv_thread_started = 1;
    }
}

static void _tcp_recv_thread_stop(tcp_t *dev)
{
    if (dev->recv_thread_started)
    {
        pthread_cancel(dev->recv_thread);
        dev->recv_thread_started = 0;
    }
}

#endif // __linux__
