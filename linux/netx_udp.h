#ifndef _NETX_UDP_H_
#define _NETX_UDP_H_

#ifdef __linux__

#include "netx.h"

enum {
    UDP_CTRL_CMD_BIND = 0,
    UDP_CTRL_CMD_MAX,
};

int32_t NetxUdpCreate(netx *self, const char *desc);

int32_t NetxUdpDestory(netx *self);

#endif // __linux__
#endif // _NETX_UDP_H_
