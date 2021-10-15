#ifndef __robin_c_http
#define __robin_c_http


enum http_method {
    HTTP_GET,
};

const _string method_strings[1] = {
    make_string("GET"),
};

struct http_request {
    s32 socket;
    s32 headers_end;
};

struct net_url {
    net_scheme  scheme;
    u16         port;
    _string    *host;
    _string    *path;
};

s32 http_start(net_url uri, http_method method) {
    ipv4 address = dns_lookup(uri.host);
    if (address == 0) return -1;

    s32 sock = net_tcp_socket(...);
    if (sock < 0) return -1;

    s32 success = net_connect(sock, address, port);
    if (success < 0) return -1;

    c8 buffer[1024];
    s8 i = 0;

    strcpy(buffer+i, method_strings[method.data], method_strings[method].length);
    i += method_strings[method].length;
    buffer[i++] = ' ';

    strcpy(buffer+i, uri.path.data, uri.path.length);
    i += uri.path_length;
    buffer[i++] = ' ';

    strcpy(buffer+i, "HTTP/1.1\r\n", 10);
}


#endif // __robin_c_http
