#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>

#include "ip.h"


typedef struct socket_address {
    u8   length;     // Total length of the struct
    u8   family;
    u16  port;       // In network byte order
    ipv4 address;
    u8   padding[8];
} socket_address;

u16 reverse_byte_order_16(u16 port) {
    return (port << 8) | (port >> 8);
}

u32 reverse_byte_order_32(u32 v) {
    return (((v & 0xFF000000) >> 24) |
            ((v & 0x00FF0000) >> 8) |
            ((v & 0x0000FF00) << 8) |
            ((v & 0x000000FF) << 24));
}

int main(int argc, char *argv[]) {
    int s = socket(PF_INET, SOCK_STREAM, 0);
    if (s == -1) return 1;

    socket_address server;
    server.length  = sizeof(socket_address);
    server.family  = AF_INET;
    server.address = reverse_byte_order_32(ipv4_parse_address("0.0.0.0"));
    server.port    = reverse_byte_order_16(8080);

    s32 success = bind(s, (struct sockaddr *) &server, sizeof(socket_address));
    if (success < 0) {
        printf("bind: %d\n", errno);
        return 1;
    }

    if (listen(s, 10) == -1) return 3;

    c8 buffer[1024];

    socket_address client;
    u32 client_length = sizeof(socket_address);
    for (;;) {
        s32 conn = accept(s, (struct sockaddr *) &client, &client_length);
        if (conn == -1) return 4;

        ipv4 client_address = reverse_byte_order_32(client.address);
        ipv4_b c = ipv4_bytes(client_address);
        u16 port = reverse_byte_order_16(client.port);
        printf("accepted: %u.%u.%u.%u:%i\n", c.abcd.a, c.abcd.b, c.abcd.c, c.abcd.d, port);

        s32 len = read(conn, buffer, 1024);
        printf("%.*s\n", len, buffer);
        break;
    }

    success = close(s);
    if (success < 0) {
        printf("close: %d\n", errno);
        return 1;
    }

    return 0;
}
