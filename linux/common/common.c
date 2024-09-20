#include "common.h"

int parse_address(const char *address_str, struct sockaddr_in *addr)
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
