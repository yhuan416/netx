#ifndef _NETX_UART_H_
#define _NETX_UART_H_

#ifdef __linux__

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int parse_address(const char *address_str, struct sockaddr_in *addr);

#endif // __linux__
#endif
