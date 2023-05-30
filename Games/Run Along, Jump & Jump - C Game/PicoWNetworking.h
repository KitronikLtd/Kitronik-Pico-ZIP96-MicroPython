/*
This code was written and modified by Jack Faulkner for Kitronik Ltd.

Large sections of the code is modified code from the Raspberry Pi 
pico-examples GitHub repository and the MicroPython project.

Thank you to both of these projects as this library wouldn't have been
possible without them.
*/

/*
MIT License

Copyright (c) 2022 Kitronik Ltd 

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
/*
Copyright (c) 2022 Raspberry Pi (Trading) Ltd.

SPDX-License-Identifier: BSD-3-Clause
*/
/*
This file is part of the MicroPython project, http://micropython.org/

The MIT License (MIT)

Copyright (c) 2018-2019 Damien P. George

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#ifndef TCPIP_H_
#define TCPIP_H_

#include "lwip/ip_addr.h"

#define DHCPS_BASE_IP 16
#define DHCPS_MAX_IP 8

typedef struct DHCPServerLease {
    uint8_t mac[6];
    uint16_t expiry;
} DHCPServerLease;

typedef struct DHCPServer {
    ip_addr_t ip;
    ip_addr_t nm;
    DHCPServerLease lease[DHCPS_MAX_IP];
    struct udp_pcb *udp;
} DHCPServer;

#define TCP_PORT 80
#define BUF_SIZE 1024

typedef struct Server {
    struct tcp_pcb *server;
    struct tcp_pcb *client;
    DHCPServer dhcpServer;
    bool connected, received, complete;
    uint8_t data[BUF_SIZE];
    uint16_t length;
} Server;

typedef struct Client {
    struct tcp_pcb *server;
    ip_addr_t remoteAddress;
    bool connected, received, complete;
    uint8_t data[BUF_SIZE];
    uint16_t length;
} Client;

Server *serverInit(void);
bool serverOpen(void *arg);
err_t serverResult(void *arg, int status);
err_t serverAccept(void *arg, struct tcp_pcb *tpcb, err_t err);
err_t serverRecieve(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
err_t serverSend(void *arg, void *data, u16_t len);
void serverError(void *arg, err_t err);
err_t serverClose(void *arg);

Client* clientInit(void);
bool clientOpen(void *arg);
err_t clientResult(void *arg, int status);
err_t clientConnected(void *arg, struct tcp_pcb *tpcb, err_t err);
err_t clientRecieve(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
err_t clientSend(void *arg, void *data, u16_t len);
void clientError(void *arg, err_t err);
err_t clientClose(void *arg);

#define DHCPDISCOVER    1
#define DHCPOFFER       2
#define DHCPREQUEST     3
#define DHCPDECLINE     4
#define DHCPACK         5
#define DHCPNACK        6
#define DHCPRELEASE     7
#define DHCPINFORM      8

#define DHCP_OPT_PAD                0
#define DHCP_OPT_SUBNET_MASK        1
#define DHCP_OPT_ROUTER             3
#define DHCP_OPT_DNS                6
#define DHCP_OPT_HOST_NAME          12
#define DHCP_OPT_REQUESTED_IP       50
#define DHCP_OPT_IP_LEASE_TIME      51
#define DHCP_OPT_MSG_TYPE           53
#define DHCP_OPT_SERVER_ID          54
#define DHCP_OPT_PARAM_REQUEST_LIST 55
#define DHCP_OPT_MAX_MSG_SIZE       57
#define DHCP_OPT_VENDOR_CLASS_ID    60
#define DHCP_OPT_CLIENT_ID          61
#define DHCP_OPT_END                255

#define PORT_DHCP_SERVER 67
#define PORT_DHCP_CLIENT 68

#define DEFAULT_DNS MAKE_IP4(8, 8, 8, 8)
#define DEFAULT_LEASE_TIME_S (24 * 60 * 60) // in seconds

#define MAC_LEN 6
#define MAKE_IP4(a, b, c, d) ((a) << 24 | (b) << 16 | (c) << 8 | (d))

typedef struct DHCPMessage {
    uint8_t op; // message opcode
    uint8_t htype; // hardware address type
    uint8_t hlen; // hardware address length
    uint8_t hops;
    uint32_t xid; // transaction id, chosen by client
    uint16_t secs; // client seconds elapsed
    uint16_t flags;
    uint8_t ciaddr[4]; // client IP address
    uint8_t yiaddr[4]; // your IP address
    uint8_t siaddr[4]; // next server IP address
    uint8_t giaddr[4]; // relay agent IP address
    uint8_t chaddr[16]; // client hardware address
    uint8_t sname[64]; // server host name
    uint8_t file[128]; // boot file name
    uint8_t options[312]; // optional parameters, variable, starts with magic
} DHCPMessage;

void dhcpServerInit(DHCPServer *d, ip_addr_t *ip, ip_addr_t *nm);
void dhcpServerDeinit(DHCPServer *d);

#endif // TCPIP_H_
