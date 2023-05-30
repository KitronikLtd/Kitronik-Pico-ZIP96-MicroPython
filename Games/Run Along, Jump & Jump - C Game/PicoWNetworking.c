/*
This code was written and modified by Kitronik Ltd.

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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#include "pico/cyw43_arch.h"
#include "pico/multicore.h"
#include "pico/stdlib.h"

#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "lwip/udp.h"
#include "cyw43_config.h"

#include "PicoWNetworking.h"

/*
*
Library for networking on the Pico W.
*
*/

/*
The server functions enable the use of the wireless access point and TCP server functionality.
*/
// Start the WAP for client to connect to.
Server *serverInit(void) {
    // Setup the Server state.
    Server *state = calloc(1, sizeof(Server));
    if (!state) {
        return NULL; // Failed to allocate state.
    }

    // Initialise the wireless chip
    if (cyw43_arch_init()) {
        return NULL; // Failed to initialise CYW43 chip.
    }

    // Setup wireless chip to WAP mode
    const char *ap_name = "Pico W AP";
    const char *password = "Kitronik1";
    cyw43_arch_enable_ap_mode(ap_name, password, CYW43_AUTH_WPA2_AES_PSK);

    // Start the DHCP server to accept wireless connections.
    ip4_addr_t gw, mask;
    IP4_ADDR(&gw, 192, 168, 4, 1);
    IP4_ADDR(&mask, 255, 255, 255, 0);
    dhcpServerInit(&(state->dhcpServer), &gw, &mask);

    return state;
}

// Start the TCP server for client to connect to.
bool serverOpen(void *arg) {
    Server *state = (Server *) arg;
    
    // Setup the TCP protocol control block for the server's IP address.
    struct tcp_pcb *pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
    if (!pcb) {
        return false; // Failed to create PCB.
    }

    // Starting server on port TCP_PORT
    err_t err = tcp_bind(pcb, NULL, TCP_PORT);
    if (err) {
        return false; // Failed to bind to port.
    }

    // Start the server listening for TCP messages.
    state->server = tcp_listen_with_backlog(pcb, 1);
    if (!state->server) {
        if (pcb) {
            tcp_close(pcb);
        }
        return false; // Failed to listen.
    }

    // Setup the server's TCP callbacks.
    tcp_arg(state->server, state);
    tcp_accept(state->server, serverAccept);

    return true;
}

// Result callback for the server functions.
err_t serverResult(void *arg, int status) {
    Server *state = (Server *) arg;
    if (status != 0) {
        return serverClose(arg);
    }
    return ERR_OK;
}

// Accept a client to connect to the server.
err_t serverAccept(void *arg, struct tcp_pcb *tpcb, err_t err) {
    Server *state = (Server *) arg;
    if (err != ERR_OK || tpcb == NULL) {
        serverResult(arg, err);
        return ERR_VAL; // Failure in accept.
    }

    // Store the client's TCP protocol control block
    state->client = tpcb;
    state->connected = true;
    // Setup TCP callbacks for the client.
    tcp_arg(tpcb, state);
    tcp_recv(tpcb, serverRecieve);
    tcp_err(tpcb, serverError);
    
    return ERR_OK;
}

// Callback for when the server receives a TCP message from the client.
err_t serverRecieve(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    Server *state = (Server *) arg;
    if (!p) {
        return serverResult(arg, -1); // No packet buffer received.
    }
    
    cyw43_arch_lwip_check();
    // When there is data in the packet.
    if (p->tot_len > 0) {
        // Copy the data into the server's state.
        state->length = p->tot_len <= BUF_SIZE ? p->tot_len : BUF_SIZE;
        pbuf_copy_partial(p, state->data, state->length, 0);
        state->received = true;
        tcp_recved(tpcb, p->tot_len);
    }

    pbuf_free(p);
    return ERR_OK;
}

// Function to send a TCP message from the server to the client.
err_t serverSend(void *arg, void *data, u16_t len) {
    Server *state = (Server *) arg;
    // Write the given data to a TCP message.
    err_t err = tcp_write(state->client, data, len, TCP_WRITE_FLAG_COPY);
    if (err != ERR_OK) {
        return serverResult(arg, -1); // Failed to write data.
    }
    return ERR_OK;    
}

// Error callback for the server functions.
void serverError(void *arg, err_t err) {
    Server *state = (Server *) arg;
    if (err != ERR_ABRT) {
        serverResult(arg, err);
    }
    state->complete = true;
}

// Function to close the TCP server and WAP.
err_t serverClose(void *arg) {
    Server *state = (Server *) arg;
    err_t err = ERR_OK;
    // Remove callbacks on the client TCP PCB.
    if (state->client != NULL) {
        tcp_arg(state->client, NULL);
        tcp_recv(state->client, NULL);
        tcp_err(state->client, NULL);
        err = tcp_close(state->client);

        if (err != ERR_OK) {
            tcp_abort(state->client);
            err = ERR_ABRT;
        }
        state->client = NULL;
    }

    // Remove callbacks on the server TCP PCB.
    if (state->server) {
        tcp_arg(state->server, NULL);
        tcp_close(state->server);
        state->server = NULL;
    }
    
    // Disconnect the WAP and shutdown the wireless chip.
    state->connected = false;
    dhcpServerDeinit(&(state->dhcpServer));
    cyw43_arch_deinit();
    return err;
}

/*
The client functions enable the connection to the server wireless access point and TCP server.
*/
// Connect to the server's WAP.
Client *clientInit(void) {
    // Setup the Client state.
    Client *state = calloc(1, sizeof(Client));
    if (!state) {
        return NULL; // Failed to allocate state.
    }

    // Initialise the wireless chip.
    if (cyw43_arch_init()) {
        return NULL; // Failed to initialise CYW43 chip.
    }

    // Setup wireless chip to station mode.
    cyw43_arch_enable_sta_mode();

    // Connect to the WAP.
    const char *ap_name = "Pico W AP";
    const char *password = "Kitronik1";
    if (cyw43_arch_wifi_connect_timeout_ms(ap_name, password, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        return NULL; // Failed to connect.
    }
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, true);
    ip4addr_aton("192.168.4.1", &state->remoteAddress);

    return state;
}

// Create a connection to the TCP server.
bool clientOpen(void *arg) {
    Client *state = (Client *) arg;
    
    // Setup the TCP protocol control block for the server.
    state->server = tcp_new_ip_type(IP_GET_TYPE(&state->remoteAddress));
    if (!state->server) {
        return false; // Failed to create PCB.
    }

    // Setup the server's TCP callbacks.
    tcp_arg(state->server, state);
    tcp_recv(state->server, clientRecieve);
    tcp_err(state->server, clientError);

    // Connnect to the server's TCP protocol control block.
    cyw43_arch_lwip_begin();
    err_t err = tcp_connect(state->server, &state->remoteAddress, TCP_PORT, clientConnected);
    cyw43_arch_lwip_end();

    return err == ERR_OK;
}

// Result callback for the client functions.
err_t clientResult(void *arg, int status) {
    Client *state = (Client *) arg;
    if (status != 0) {
        clientClose(arg);
    }
    return ERR_OK;
}

// Connected callback for when the client has connected to the server.
err_t clientConnected(void *arg, struct tcp_pcb *tpcb, err_t err) {
    Client *state = (Client*)arg;
    if (err != ERR_OK) {
        return clientResult(arg, err); // Connect failed.
    }

    state->connected = true;
    // Waiting for buffer from server.
    return ERR_OK;
}

// Callback for when the client receives a TCP message from the server.
err_t clientRecieve(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    Client *state = (Client *) arg;
    if (!p) {
        return clientResult(arg, -1); // No packet buffer received.
    }
    
    cyw43_arch_lwip_check();
    // When there is data in the packet.
    if (p->tot_len > 0) {
        // Copy the data into the client's state.
        state->length = p->tot_len <= BUF_SIZE ? p->tot_len : BUF_SIZE;
        pbuf_copy_partial(p, state->data, state->length, 0);
        state->received = true;
        tcp_recved(tpcb, p->tot_len);
    }

    pbuf_free(p);
    return ERR_OK;
}

// Function to send a TCP message from the client to the server.
err_t clientSend(void *arg, void *data, u16_t len) {
    Client *state = (Client *) arg;
    // Write the given data to a TCP message.
    err_t err = tcp_write(state->server, data, len, TCP_WRITE_FLAG_COPY);
    if (err != ERR_OK) {
        return clientResult(arg, -1); // Failed to write data.
    }
    return ERR_OK;
}

// Error callback for the client functions.
void clientError(void *arg, err_t err) {
    Client *state = (Client *) arg;
    if (err != ERR_ABRT) {
        clientResult(arg, err);
    }
    state->complete = true;
}

// Function to close the connection to the TCP server and WAP.
err_t clientClose(void *arg) {
    Client *state = (Client *) arg;
    err_t err = ERR_OK;
    // Remove callbacks on the server TCP PCB.
    if (state->server != NULL) {
        tcp_arg(state->server, NULL);
        tcp_recv(state->server, NULL);
        tcp_err(state->server, NULL);
        err = tcp_close(state->server);

        if (err != ERR_OK) {
            tcp_abort(state->server);
            err = ERR_ABRT;
        }
        state->server = NULL;
    }

    // Shutdown the wireless chip.
    state->connected = false;
    cyw43_arch_deinit();
    return err;
}

/*
The DHCP functions enable the server to act as a wireless access point.
*/
static int dhcp_socket_new_dgram(struct udp_pcb **udp, void *cb_data, udp_recv_fn cb_udp_recv) {
    *udp = udp_new();
    if (*udp == NULL) {
        return -ENOMEM;
    }

    // Register callback
    udp_recv(*udp, cb_udp_recv, (void *)cb_data);
    return 0;
}

static int dhcp_socket_bind(struct udp_pcb **udp, uint32_t ip, uint16_t port) {
    ip_addr_t addr;
    IP4_ADDR(&addr, ip >> 24 & 0xff, ip >> 16 & 0xff, ip >> 8 & 0xff, ip & 0xff);
    return udp_bind(*udp, &addr, port);
} 

static void dhcp_socket_free(struct udp_pcb **udp) {
    if (*udp != NULL) {
        udp_remove(*udp);
        *udp = NULL;
    }
}

static int dhcp_socket_sendto(struct udp_pcb **udp, const void *buf, size_t len, uint32_t ip, uint16_t port) {
    if (len > 0xffff) {
        len = 0xffff;
    }

    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_RAM);
    if (p == NULL) {
        return -ENOMEM;
    }

    memcpy(p->payload, buf, len);
    ip_addr_t dest;
    IP4_ADDR(&dest, ip >> 24 & 0xff, ip >> 16 & 0xff, ip >> 8 & 0xff, ip & 0xff);
    err_t err = udp_sendto(*udp, p, &dest, port);
    pbuf_free(p);

    if (err != ERR_OK) {
        return err;
    }

    return len;
}

static uint8_t *opt_find(uint8_t *opt, uint8_t cmd) {
    for (int i = 0; i < 308 && opt[i] != DHCP_OPT_END;) {
        if (opt[i] == cmd) {
            return &opt[i];
        }
        i += 2 + opt[i + 1];
    }
    return NULL;
}

static void opt_write_n(uint8_t **opt, uint8_t cmd, size_t n, void *data) {
    uint8_t *o = *opt;
    *o++ = cmd;
    *o++ = n;
    memcpy(o, data, n);
    *opt = o + n;
}

static void opt_write_u8(uint8_t **opt, uint8_t cmd, uint8_t val) {
    uint8_t *o = *opt;
    *o++ = cmd;
    *o++ = 1;
    *o++ = val;
    *opt = o;
}

static void opt_write_u32(uint8_t **opt, uint8_t cmd, uint32_t val) {
    uint8_t *o = *opt;
    *o++ = cmd;
    *o++ = 4;
    *o++ = val >> 24;
    *o++ = val >> 16;
    *o++ = val >> 8;
    *o++ = val;
    *opt = o;
}

static void dhcpServerInitprocess(void *arg, struct udp_pcb *upcb, struct pbuf *p, const ip_addr_t *src_addr, u16_t src_port) {
    DHCPServer *d = arg;
    (void)upcb;
    (void)src_addr;
    (void)src_port;
    DHCPMessage dhcp_msg;

    #define DHCP_MIN_SIZE (240 + 3)
    if (p->tot_len < DHCP_MIN_SIZE) {
        goto ignore_request;
    }

    size_t len = pbuf_copy_partial(p, &dhcp_msg, sizeof(dhcp_msg), 0);
    if (len < DHCP_MIN_SIZE) {
        goto ignore_request;
    }

    dhcp_msg.op = DHCPOFFER;
    memcpy(&dhcp_msg.yiaddr, &d->ip.addr, 4);
    uint8_t *opt = (uint8_t *)&dhcp_msg.options;
    opt += 4; // assume magic cookie: 99, 130, 83, 99

    switch (opt[2]) {
        case DHCPDISCOVER: {
            int yi = DHCPS_MAX_IP;
            for (int i = 0; i < DHCPS_MAX_IP; ++i) {
                if (memcmp(d->lease[i].mac, dhcp_msg.chaddr, MAC_LEN) == 0) {
                    // MAC match, use this IP address
                    yi = i;
                    break;
                }
                if (yi == DHCPS_MAX_IP) {
                    // Look for a free IP address
                    if (memcmp(d->lease[i].mac, "\x00\x00\x00\x00\x00\x00", MAC_LEN) == 0) {
                        // IP available
                        yi = i;
                    }
                    uint32_t expiry = d->lease[i].expiry << 16 | 0xffff;
                    if ((int32_t)(expiry - cyw43_hal_ticks_ms()) < 0) {
                        // IP expired, reuse it
                        memset(d->lease[i].mac, 0, MAC_LEN);
                        yi = i;
                    }
                }
            }
            if (yi == DHCPS_MAX_IP) {
                // No more IP addresses left
                goto ignore_request;
            }
            dhcp_msg.yiaddr[3] = DHCPS_BASE_IP + yi;
            opt_write_u8(&opt, DHCP_OPT_MSG_TYPE, DHCPOFFER);
            break;
        }

        case DHCPREQUEST: {
            uint8_t *o = opt_find(opt, DHCP_OPT_REQUESTED_IP);
            if (o == NULL) {
                // Should be NACK
                goto ignore_request;
            }
            if (memcmp(o + 2, &d->ip.addr, 3) != 0) {
                // Should be NACK
                goto ignore_request;
            }
            uint8_t yi = o[5] - DHCPS_BASE_IP;
            if (yi >= DHCPS_MAX_IP) {
                // Should be NACK
                goto ignore_request;
            }
            if (memcmp(d->lease[yi].mac, dhcp_msg.chaddr, MAC_LEN) == 0) {
                // MAC match, ok to use this IP address
            } else if (memcmp(d->lease[yi].mac, "\x00\x00\x00\x00\x00\x00", MAC_LEN) == 0) {
                // IP unused, ok to use this IP address
                memcpy(d->lease[yi].mac, dhcp_msg.chaddr, MAC_LEN);
            } else {
                // IP already in use
                // Should be NACK
                goto ignore_request;
            }
            d->lease[yi].expiry = (cyw43_hal_ticks_ms() + DEFAULT_LEASE_TIME_S * 1000) >> 16;
            dhcp_msg.yiaddr[3] = DHCPS_BASE_IP + yi;
            opt_write_u8(&opt, DHCP_OPT_MSG_TYPE, DHCPACK);
            printf("DHCPS: client connected: MAC=%02x:%02x:%02x:%02x:%02x:%02x IP=%u.%u.%u.%u\n",
                dhcp_msg.chaddr[0], dhcp_msg.chaddr[1], dhcp_msg.chaddr[2], dhcp_msg.chaddr[3], dhcp_msg.chaddr[4], dhcp_msg.chaddr[5],
                dhcp_msg.yiaddr[0], dhcp_msg.yiaddr[1], dhcp_msg.yiaddr[2], dhcp_msg.yiaddr[3]);
            break;
        }

        default:
            goto ignore_request;
    }

    opt_write_n(&opt, DHCP_OPT_SERVER_ID, 4, &d->ip.addr);
    opt_write_n(&opt, DHCP_OPT_SUBNET_MASK, 4, &d->nm.addr);
    opt_write_n(&opt, DHCP_OPT_ROUTER, 4, &d->ip.addr); // aka gateway; can have mulitple addresses
    opt_write_u32(&opt, DHCP_OPT_DNS, DEFAULT_DNS); // can have mulitple addresses
    opt_write_u32(&opt, DHCP_OPT_IP_LEASE_TIME, DEFAULT_LEASE_TIME_S);
    *opt++ = DHCP_OPT_END;
    dhcp_socket_sendto(&d->udp, &dhcp_msg, opt - (uint8_t *)&dhcp_msg, 0xffffffff, PORT_DHCP_CLIENT);

ignore_request:
    pbuf_free(p);
}

// Start the DHCP server to act as a wireless access point.
void dhcpServerInit(DHCPServer *d, ip_addr_t *ip, ip_addr_t *nm) {
    ip_addr_copy(d->ip, *ip);
    ip_addr_copy(d->nm, *nm);
    memset(d->lease, 0, sizeof(d->lease));
    if (dhcp_socket_new_dgram(&d->udp, d, dhcpServerInitprocess) != 0) {
        return;
    }
    dhcp_socket_bind(&d->udp, 0, PORT_DHCP_SERVER);
}

// Stop the DHCP server to remove the wireless access point.
void dhcpServerDeinit(DHCPServer *d) {
    dhcp_socket_free(&d->udp);
}
