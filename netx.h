#ifndef _NETX_H_
#define _NETX_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

typedef struct _netx netx;

typedef int32_t (*netx_on_event)(netx *self, uint32_t event, void *data, uint32_t len);
typedef int32_t (*netx_on_data)(netx *self, void *data, uint32_t len);

typedef int32_t (*netx_start)(netx *self, netx_on_data on_data);

typedef int32_t (*netx_stop)(netx *self);

typedef int32_t (*netx_send)(netx *self, const uint8_t *data, uint32_t len);

typedef int32_t (*netx_ctrl)(netx *self, uint32_t cmd, void *data, uint32_t len);

typedef int32_t (*netx_get_mtu)(netx *self);

typedef struct
{
    netx_start start;
    netx_stop stop;

    netx_send send;

    netx_ctrl ctrl;
    netx_get_mtu get_mtu;
} netx_interface_t;

enum
{
    NETX_INIT = 0,
    NETX_START,
    NETX_STOP,
};

struct _netx
{
    netx_interface_t *interface;
    uint32_t state;

    netx_on_event on_event;
    netx_on_data on_data;

    void *priv;
};

int32_t NetxStart(netx *self, netx_on_data on_data);

int32_t NetxStop(netx *self);

int32_t NetxSend(netx *self, const uint8_t *data, uint32_t len);

int32_t NetxRegOnEvent(netx *self, netx_on_event on_event);

int16_t NetxGetMtu(netx *self);

// for lower layer to call
int32_t NetxOnEvent(netx *self, uint32_t event, void *data, uint32_t len);

int32_t NetxOnData(netx *self, void *data, uint32_t len);

#endif
