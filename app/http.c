// #include <sys/socket.h>
// #include <unistd.h>
// #include <errno.h>

#include "ip.h"
#include "net.h"
#include "string.h"
#include "string_builder.h"


s32 main(s32 argc, c8 *argv[]) {

    //
    // req, err := http.NewRequest("GET", s.URL().String(), nil)
    // if err != nil {
    // 	return "", err
    // }
    // req.Header.Add("Accept", acceptHeader)
    // req.Header.Add("Accept-Encoding", "gzip")
    // req.Header.Set("User-Agent", UserAgent)
    // req.Header.Set("X-Prometheus-Scrape-Timeout-Seconds", strconv.FormatFloat(s.timeout.Seconds(), 'f', -1, 64))
    //

    ipv4 addr     = ipv4_parse_address("127.0.0.1");
    net_conn conn = net_connect(NET_TCP, addr, 9090);
    if (conn.socket < 0) {
        return 1;
    }

    string_builder *builder = string_make_builder();
    string_write(builder, "GET /metrics HTTP/1.1\r\n");
    string_write(builder, "Host: 127.0.0.1:9090\r\n");
    string_write(builder, "Accept: application/openmetrics-text; version=0.0.1,text/plain;version=0.0.4;q=0.5,*/*;q=0.1\r\n");
    string_write(builder, "User-Agent: Crometheus/0.0.0\r\n");
    string_write(builder, "\r\n");

    string request = string_builder_to_string(builder);
    string_free_builder(builder);
    printf("Request:\n\n%.*s\n\n", request.length, request.data);

    s32 sent = send(conn.socket, request.data, request.length, 0);
    if (sent != request.length) {
        close(conn.socket);
        return 1;
    }

    c8 buffer[1024];
    builder = string_make_builder();
    s32 received = 1024;
    while (received == 1024) {
        received = recv(conn.socket, buffer, 1024, 0);
        string_write_n(builder, buffer, received);
    }
    string response = string_builder_to_string(builder);
    string_free_builder(builder);
    printf("Response:\n\n%.*s\n", response.length, response.data);

    return 0;
}
