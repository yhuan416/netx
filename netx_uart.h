#ifndef _NETX_UART_H_
#define _NETX_UART_H_

#include "netx.h"

typedef enum {
    UART_ON_DATA = 0,
    UART_ON_EVENT_MAX,
} UART_EVENT;

int32_t NetxUartCreate(netx *self, const char *desc);

int32_t NetxUartDestory(netx *self);

#endif
