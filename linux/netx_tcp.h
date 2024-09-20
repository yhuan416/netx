#ifndef _NETX_TCP_H_
#define _NETX_TCP_H_

#ifdef __linux__

#include "netx.h"

enum {
    TCP_CTRL_CMD_BIND = 0,
    TCP_CTRL_CMD_MAX,
};

int32_t NetxTcpCreate(netx *self, const char *desc);

int32_t NetxTcpDestory(netx *self);

#endif // __linux__
#endif // _NETX_TCP_H_
