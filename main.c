#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "netx.h"
#include "netx_uart.h"
#include "netx_udp.h"

int32_t on_event(netx *self, uint32_t event, void *data, uint32_t len)
{
    printf("on_event, %d\n", event);

    if (event == UART_ON_DATA)
    {
        printf("len: %d\n", len);
        printf("data: %s\n", (char *)data);
    }
    else if (event == UART_ON_CLOSE)
    {
        printf("uart closed.\n");
    }

    return 0;
}

int32_t on_data(netx *self, void *data, uint32_t len, void *extend, uint32_t ext_len)
{
    printf("on_data\n");
    printf("len: %d\n", len);
    printf("data: %s\n", (char *)data);
    return 0;
}

int main(int argc, char const *argv[])
{
    int i;
    int32_t ret = -1;
    netx _netx = {0};

#if 0
    ret = NetxUartCreate(&_netx, argv[1]);
    if (ret < 0)
    {
        printf("NetxUartCreate failed\n");
        return -1;
    }
#else
    ret = NetxUdpCreate(&_netx, argv[1]);
    if (ret < 0)
    {
        printf("NetxUdpCreate failed\n");
        return -1;
    }
#endif

    if (NetxStart(&_netx, on_data) < 0)
    {
        printf("NetxStart failed\n");
        return -1;
    }

    if (argc > 2)
    {
        if (NetxCtrl(&_netx, UDP_CTRL_CMD_BIND, (void *)argv[2], strlen(argv[2])) < 0)
        {
            printf("NetxCtrl failed\n");
            return -1;
        }
    }

    for (i = 0; i < 10; i++)
    {
        sleep(1);
        if (NetxSend(&_netx, (const uint8_t *)"hello", strlen("hello")) < 0)
        {
            printf("NetxSend failed\n");
            return -1;
        }
    }

    NetxStop(&_netx);

#if 0
    if (NetxUartDestory(&_netx) < 0)
    {
        printf("NetxUartDestory failed\n");
        return -1;
    }
#else
    if (NetxUdpDestory(&_netx) < 0)
    {
        printf("NetxUdpDestory failed\n");
        return -1;
    }
#endif

    return 0;
}
