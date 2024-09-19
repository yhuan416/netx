#include "netx_uart.h"

#define _GNU_SOURCE // 在源文件开头定义_GNU_SOURCE 宏
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <termios.h>
#include <sys/epoll.h>
#include <pthread.h>

#ifndef MAX_DEVICE_PATH_LENGTH
#define MAX_DEVICE_PATH_LENGTH (32)
#endif

#define MAX_UART_DEVICE_NUM (8)

typedef struct uart_cfg
{
    char path[MAX_DEVICE_PATH_LENGTH];
    unsigned int baudrate; /* 波特率 */
    unsigned char dbit;    /* 数据位 */
    char parity;           /* 奇偶校验 */
    unsigned char sbit;    /* 停止位 */
} uart_cfg_t;

typedef struct
{
    uart_cfg_t cfg;
    int fd;

    pthread_t uart_recv_thread;
    int uart_recv_thread_started;

    void *priv;
} uart_dev_t;

static int g_uart_dev_index = 0;
static uart_dev_t g_uart_dev[MAX_UART_DEVICE_NUM] = {0};

static int _UartDesc2Cfg(const char *_desc, uart_cfg_t *cfg);
static int uart_cfg(int fd, const uart_cfg_t *cfg);

static uart_dev_t *alloc_uart_dev()
{
    uart_dev_t *dev = NULL;
    if (g_uart_dev_index < 8)
    {
        dev = &g_uart_dev[g_uart_dev_index];
        g_uart_dev_index++;
    }
    return dev;
}

void *uart_recv_thread(void *arg)
{
    uart_dev_t *dev = (uart_dev_t *)arg;
    int fd = dev->fd;
    uint8_t buf[1500] = {0};

    printf("uart_recv_thread started\n");

    while (1)
    {
        memset(buf, 0, sizeof(buf));
        int len = read(fd, buf, sizeof(buf));
        if (len > 0)
        {
            NetxOnEvent(dev->priv, UART_ON_DATA, buf, len);
        }
    }
}

static void _uart_recv_thread_start(uart_dev_t *dev)
{
    if (!dev->uart_recv_thread_started)
    {
        pthread_create(&dev->uart_recv_thread, NULL, uart_recv_thread, (void *)dev);
        dev->uart_recv_thread_started = 1;
    }
}

static netx_interface_t g_uart_netx_interface;

int32_t NetxUartCreate(netx *self, const char *desc)
{
    int32_t ret = -1;
    uart_dev_t *dev = alloc_uart_dev();

    if (!self || !dev)
    {
        return ret;
    }

    memset(dev, 0, sizeof(uart_dev_t));

    if (_UartDesc2Cfg(desc, &dev->cfg) != 0)
    {
        return ret;
    }

    dev->priv = self;

    self->interface = &g_uart_netx_interface;
    self->priv = dev;
    ret = 0;

    return ret;
}

int32_t NetxUartDestory(netx *self)
{
    int32_t ret = -1;
    uart_dev_t *dev = NULL;

    if (!self)
        return -1;

    dev = (uart_dev_t *)self->priv;
    if (dev)
    {
        close(dev->fd);
    }

    self->interface = NULL;
    self->priv = NULL;
    ret = 0;

    return ret;
}

static int32_t _uart_netx_send(netx *self, const uint8_t *data, uint32_t len)
{
    uart_dev_t *dev = (uart_dev_t *)self->priv;
    int32_t ret = -1;
    if (dev)
    {
        ret = write(dev->fd, data, len);
    }
    return ret;
}

static int32_t _uart_netx_recv_start(netx *self)
{
    int32_t ret = 0;

    uart_dev_t *dev = (uart_dev_t *)self->priv;
    if (dev)
    {
        _uart_recv_thread_start(dev);
    }

    return ret;
}

static int32_t _uart_netx_get_mtu(netx *self)
{
    return 1500;
}

static int32_t _uart_netx_ctrl(netx *self, uint32_t cmd, void *data, uint32_t len)
{
    int32_t ret = 0;
    return ret;
}

int32_t _uart_netx_start(netx *self)
{
    int fd;
    int32_t ret = 0;

    uart_dev_t *dev = (uart_dev_t *)self->priv;

    fd = open(dev->cfg.path, O_RDWR | O_NOCTTY);
    if (fd < 0)
    {
        return ret;
    }

    if (uart_cfg(fd, &dev->cfg) != 0)
    {
        close(fd);
        return ret;
    }

    dev->fd = fd;
    return ret;
}

int32_t _uart_netx_stop(netx *self)
{
    int32_t ret = 0;
    uart_dev_t *dev = (uart_dev_t *)self->priv;
    if (dev)
    {
        NetxOnEvent(dev->priv, UART_ON_CLOSE, NULL, 0);

        close(dev->fd);
        dev->fd = -1;
    }
    return ret;
}

static netx_interface_t g_uart_netx_interface = {
    .start = _uart_netx_start,
    .stop = _uart_netx_stop,

    .send = _uart_netx_send,

    .recv_start = _uart_netx_recv_start,

    .get_mtu = _uart_netx_get_mtu,
    .ctrl = _uart_netx_ctrl,
};

static char *_strncpy(char *dest, const char *src, size_t n)
{
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; i++)
        dest[i] = src[i];
    for (; i < n; i++)
        dest[i] = '\0';

    return dest;
}

static void _memset(void *s, char c, size_t n)
{
    unsigned char *p = (unsigned char *)s;
    while (n--)
    {
        *p++ = (unsigned char)c;
    }
}

static int _UartDesc2Cfg(const char *_desc, uart_cfg_t *cfg)
{
    char *token;
    int ret = 0;
    char desc[64] = {0};
    long _tmp;

    // 复制一份描述字符串
    _strncpy(desc, _desc, sizeof(desc));

    // 初始化配置
    _memset(cfg, 0, sizeof(uart_cfg_t));

    // 解析设备路径
    token = strtok(desc, ":");
    if (token == NULL)
    {
        ret = -1;
        goto cleanup;
    }
    strncpy(cfg->path, token, MAX_DEVICE_PATH_LENGTH);

    // 设置默认值
    cfg->baudrate = 115200;
    cfg->dbit = 8;
    cfg->parity = 'N';
    cfg->sbit = 1;

    // 解析后续选项
    while ((token = strtok(NULL, ":")) != NULL)
    {
        if (token[0] == 'B')
        {
            _tmp = atoi(token + 1);
            if (_tmp)
            {
                cfg->baudrate = _tmp;
            }
        }
        else if (token[0] == 'D')
        {
            _tmp = atoi(token + 1);
            if (_tmp)
            {
                cfg->dbit = _tmp;
            }
        }
        else if (strlen(token) == 1)
        {
            // 奇偶校验位, 取值 N O E
            if (token[0] == 'O' || token[0] == 'E')
            {
                cfg->parity = token[0];
            }
        }
        else if (token[0] == 'S')
        {
            _tmp = atoi(token + 1);
            if (_tmp)
            {
                cfg->sbit = _tmp;
            }
        }
    }

cleanup:
    return ret;
}

static int uart_cfg(int fd, const uart_cfg_t *cfg)
{
    struct termios new_cfg = {0}; // 将 new_cfg 对象清零
    speed_t speed;
    /* 设置为原始模式 */
    cfmakeraw(&new_cfg);
    /* 使能接收 */
    new_cfg.c_cflag |= CREAD;
    /* 设置波特率 */
    switch (cfg->baudrate)
    {
    case 1200:
        speed = B1200;
        break;
    case 1800:
        speed = B1800;
        break;
    case 2400:
        speed = B2400;
        break;
    case 4800:
        speed = B4800;
        break;
    case 9600:
        speed = B9600;
        break;
    case 19200:
        speed = B19200;
        break;
    case 38400:
        speed = B38400;
        break;
    case 57600:
        speed = B57600;
        break;
    case 115200:
        speed = B115200;
        break;
    case 230400:
        speed = B230400;
        break;
    case 460800:
        speed = B460800;
        break;
    case 500000:
        speed = B500000;
        break;
    default: // 默认配置为 115200
        speed = B115200;
        printf("default baud rate: 115200\n");
        break;
    }
    if (0 > cfsetspeed(&new_cfg, speed))
    {
        fprintf(stderr, "cfsetspeed error: %s\n", strerror(errno));
        return -1;
    }
    /* 设置数据位大小 */
    new_cfg.c_cflag &= ~CSIZE; // 将数据位相关的比特位清零
    switch (cfg->dbit)
    {
    case 5:
        new_cfg.c_cflag |= CS5;
        break;
    case 6:
        new_cfg.c_cflag |= CS6;
        break;
    case 7:
        new_cfg.c_cflag |= CS7;
        break;
    case 8:
        new_cfg.c_cflag |= CS8;
        break;
    default: // 默认数据位大小为 8
        new_cfg.c_cflag |= CS8;
        printf("default data bit size: 8\n");
        break;
    }
    /* 设置奇偶校验 */
    switch (cfg->parity)
    {
    case 'N': // 无校验
        new_cfg.c_cflag &= ~PARENB;
        new_cfg.c_iflag &= ~INPCK;
        break;
    case 'O': // 奇校验
        new_cfg.c_cflag |= (PARODD | PARENB);
        new_cfg.c_iflag |= INPCK;
        break;
    case 'E': // 偶校验
        new_cfg.c_cflag |= PARENB;
        new_cfg.c_cflag &= ~PARODD; /* 清除 PARODD 标志，配置为偶校验 */
        new_cfg.c_iflag |= INPCK;
        break;
    default: // 默认配置为无校验
        new_cfg.c_cflag &= ~PARENB;
        new_cfg.c_iflag &= ~INPCK;
        printf("default parity: N\n");
        break;
    }
    /* 设置停止位 */
    switch (cfg->sbit)
    {
    case 1: // 1 个停止位
        new_cfg.c_cflag &= ~CSTOPB;
        break;
    case 2: // 2 个停止位
        new_cfg.c_cflag |= CSTOPB;
        break;
    default: // 默认配置为 1 个停止位
        new_cfg.c_cflag &= ~CSTOPB;
        printf("default stop bit size: 1\n");
        break;
    }
    /* 将 MIN 和 TIME 设置为 0 */
    new_cfg.c_cc[VTIME] = 0;
    new_cfg.c_cc[VMIN] = 0;
    /* 清空缓冲区 */
    if (0 > tcflush(fd, TCIOFLUSH))
    {
        fprintf(stderr, "tcflush error: %s\n", strerror(errno));
        return -1;
    }
    /* 写入配置、使配置生效 */
    if (0 > tcsetattr(fd, TCSANOW, &new_cfg))
    {
        fprintf(stderr, "tcsetattr error: %s\n", strerror(errno));
        return -1;
    }
    /* 配置 OK 退出 */
    return 0;
}
