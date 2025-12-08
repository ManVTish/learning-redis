#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

// struct in_addr {
//     uint32_t s_addr; // IPv4 in big-endian format
// };

// struct stockaddr_in {
//     uint16_t sin_family; // Address family (e.g., AF_INET)
//     uint16_t sin_port;  // Port number in big-endian format
//     struct in_addr sin_addr;  // IP address (e.g., IPv4)
// };

static void die(const char *msg) {
    int error = errno;
    fprintf(stderr, "[%d] %s\n", error, msg);
    abort();
}

int main() {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        die("socket()");
    }

    struct sockaddr_in srv_addr = {};
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = ntohs(1234);
    srv_addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK); // 127.0.0.1

    int rv = connect(fd, (const sockaddr *)&srv_addr, sizeof(srv_addr));
    if (rv) {
        die("connect");
    }
    printf("Connected to server✌️ \n");

    char msg[] = "Hello from client";
    write(fd, msg, strlen(msg));
    printf("Sent to server \n");

    char rbuf[64] = {};
    ssize_t n = read(fd, rbuf, sizeof(rbuf)-1);
    if (n < 0) {
        die("read");
    }
    printf("Received from server: %s\n", rbuf);
    close(fd);

    return 0;
}
