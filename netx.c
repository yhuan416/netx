#include "netx.h"

static int32_t _netx_on_event_default(netx *self, uint32_t event, void *data, uint32_t len)
{
    printf("_netx_on_event_default event, %d, data: %p, len: %d\n", event, data, len);
    return 0;
}

static int32_t _netx_on_data_default(netx *self, void *data, uint32_t len)
{
    printf("_netx_on_data_default data: %p, len: %d\n", data, len);
    return 0;
}

int32_t NetxStart(netx *self, netx_on_data on_data)
{
    int32_t ret = -1;

    if (!self)
    {
        return ret;
    }

    if (self->state == NETX_START)
    {
        return 0;
    }

    // save on_data callback
    self->on_data = on_data;

    if (self->interface->start)
    {
        ret = self->interface->start(self, on_data);
        if (ret == 0)
        {
            self->state = NETX_START;
        }
    }
    return ret;
}

int32_t NetxStop(netx *self)
{
    int32_t ret = -1;
    if (self->interface->stop)
    {
        ret = self->interface->stop(self);
        if (ret == 0)
        {
            self->state = NETX_INIT;
        }
    }
    return ret;
}

int32_t NetxSend(netx *self, const uint8_t *data, uint32_t len)
{
    int32_t ret = -1;
    if (self->interface->send)
    {
        ret = self->interface->send(self, data, len);
    }
    return ret;
}

int32_t NetxRegOnEvent(netx *self, netx_on_event on_event)
{
    int32_t ret = -1;

    if (self)
    {
        if (!on_event)
        {
            on_event = _netx_on_event_default;
        }

        self->on_event = on_event;
        ret = 0;
    }
    return ret;
}

int16_t NetxCtrl(netx *self, uint32_t cmd, void *data, uint32_t len)
{
    int16_t ret = -1;
    if (self->interface->ctrl)
    {
        ret = self->interface->ctrl(self, cmd, data, len);
    }
    return ret;
}

int16_t NetxGetMtu(netx *self)
{
    int16_t ret = -1;
    if (self->interface->get_mtu)
    {
        ret = self->interface->get_mtu(self);
    }
    return ret;
}

int32_t NetxOnEvent(netx *self, uint32_t event, void *data, uint32_t len)
{
    int32_t ret = -1;
    netx_on_event on_event = _netx_on_event_default;
    if (self->on_event)
    {
        on_event = self->on_event;
    }
    ret = on_event(self, event, data, len);
    return ret;
}

int32_t NetxOnData(netx *self, void *data, uint32_t len)
{
    int32_t ret = -1;
    netx_on_data on_data = _netx_on_data_default;
    if (self->on_data)
    {
        on_data = self->on_data;
    }
    ret = on_data(self, data, len);
    return ret;
}
