#ifndef __robin_c_net_util
#define __robin_c_net_util


#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/ip.h>

#include "c.h"
#include "ip.h"
#include "string.h"
#include "string_builder.h"


//
// Declarations
//


typedef struct net_conn   net_conn;
typedef enum net_protocol net_protocol;


external net_conn net_connect(net_protocol proto, ipv4 address, u16 port);

external u16 net_reverse_bytes_16(u16 v);
external u32 net_reverse_bytes_32(u32 v);


//
// Definitions
//


enum net_protocol {
    NET_TCP,
    NET_UDP,
};

struct net_conn {
    net_protocol proto;
    s32          socket;
    ipv4         addr;
    u16          port;
};

struct net__socket_address {
    u8   length;     // Total length of the struct
    u8   family;
    u16  port;       // In network byte order
    ipv4 address;
    u8   padding[8];
};


net_conn net_connect(net_protocol proto, ipv4 address, u16 port) {
    net_conn conn = (net_conn){
        .proto  = proto,
        .addr   = address,
        .port   = port,
        .socket = -1,
    };

    s32 sock;
    switch (proto) {
        case NET_TCP:
            sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
            break;
        case NET_UDP:
            sock = socket(PF_INET, SOCK_STREAM, IPPROTO_UDP);
            break;
        default:
            conn.socket = -1;
            return conn;
    }

    if (sock == -1) {
        conn.socket = -1;
        return conn;
    }

    struct net__socket_address server;
    server.length  = sizeof(struct net__socket_address);
    server.family  = AF_INET;
    server.address = net_reverse_bytes_32(address);
    server.port    = net_reverse_bytes_16(port);
    if (connect(sock, (struct sockaddr *) &server, sizeof(struct net__socket_address)) < 0) {
        conn.socket = -1;
        return conn;
    }

    conn.socket = sock;
    return conn;
}

u16 net_reverse_bytes_16(u16 v) {
    return (v << 8) | (v >> 8);
}

u32 net_reverse_bytes_32(u32 v) {
    return (((v & 0xFF000000) >> 24) |
            ((v & 0x00FF0000) >> 8) |
            ((v & 0x0000FF00) << 8) |
            ((v & 0x000000FF) << 24));
}


/*
// @Cleanup

struct net__conn {
    s32  fd;
    ipv4 client_address;
    u16  client_port;
};

s32 net__handle_conn(struct net__conn conn) {
    c8 buffer[1024];

    s32 len = read(conn, buffer, 1024);
    while (len > 0) {
        char c;
        for (s32 i = 0; i < len; i++) {
            c = buffer[i];
        }
        len = read(conn, buffer, 1024);
    }

    struct net__request req = net__parse_request(conn);
    printf("%.*s\n", len, buffer);

    if (close(s) < 0)
        return -1;

    return 0;
}

s32 net_serve(ipv4 address, u16 port) {
    int s = socket(PF_INET, SOCK_STREAM, 0);
    if (s == -1) return 1;

    struct net__socket_address server;
    server.length  = sizeof(struct net__socket_address);
    server.family  = AF_INET;
    server.address = net_reverse_bytes_32(address);
    server.port    = net_reverse_bytes_16(port);

    if (bind(s, (struct sockaddr *) &server, sizeof(struct net__socket_address)) < 0)
        return -1;

    // @Cleanup: set queue length
    if (listen(s, 10) == -1)
        return -1;

    struct net__socket_address client;
    u32 client_length = sizeof(struct net__socket_address);

    s32 conn_fd = accept(s, (struct sockaddr *) &client, &client_length);
    if (conn_fd < 0) return -1;

    struct net__conn conn;
    conn.fd             = conn_fd;
    conn.client_address = reverse_byte_order_32(client.address);
    conn.client_port    = reverse_byte_order_16(client.port);

    return net__handle_conn(conn);
}
*/


#endif // __robin_c_net
