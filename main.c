#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "netx.h"
#include "netx_uart.h"

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

int main(int argc, char const *argv[])
{
    int i;
    int32_t ret = -1;
    netx netx_uart = {0};

    ret = NetxUartCreate(&netx_uart, argv[1]);
    if (ret < 0)
    {
        printf("NetxUartCreate failed\n");
        return -1;
    }

    if (NetxRegOnEvent(&netx_uart, on_event) < 0)
    {
        printf("NetxRegOnEvent failed\n");
        return -1;
    }

    if (NetxStart(&netx_uart) < 0)
    {
        printf("NetxStart failed\n");
        return -1;
    }

    if (NetxRecvStart(&netx_uart) < 0)
    {
        printf("NetxRecvStart failed\n");
        return -1;
    }

    for (i = 0; i < 10; i++)
    {
        sleep(1);
        if (NetxSend(&netx_uart, (const uint8_t *)"hello", strlen("hello")) < 0)
        {
            printf("NetxSend failed\n");
            return -1;
        }
    }

    NetxStop(&netx_uart);

    if (NetxUartDestory(&netx_uart) < 0)
    {
        printf("NetxUartDestory failed\n");
        return -1;
    }

    return 0;
}
