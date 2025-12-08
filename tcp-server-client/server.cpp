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

// struct sockaddr_in {
//     uint16_t sin_family; // Address family (e.g., AF_INET)
//     uint16_t sin_port;  // Port number in big-endian format
//     struct in_addr sin_addr;  // IP address (e.g., IPv4)
// };

static void die(const char *msg) {
    int error = errno;
    fprintf(stderr, "[%d] %s\n", error, msg);
    abort();
}

static void msg(const char *msg) {
    fprintf(stderr, "%s\n", msg);
}

static void do_something(int connfd){
    // one read
    char rbuf[64] = {};
    ssize_t n = read(connfd, rbuf, sizeof(rbuf)-1);
    if(n < 0){
        msg("read() error");
        return;
    }
    fprintf(stdout, "Received from client: %s\n", rbuf);

    // one write (no error handling for brevity)
    char wbuf[] = "Hello from server";
    write(connfd, wbuf, strlen(wbuf));
}

int main() {
    /* s1: obtain socket handler
    AF_INET: IPv4 Internet protocols
    SOCK_STREAM: TCP*/
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    // s2: set socket options
    int val = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    // s3: bind socket to address and port
    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(1234); // port
    addr.sin_addr.s_addr = htonl(0); // IP 0.0.0.0
    int rv = bind(fd, (const struct sockaddr *)&addr, sizeof(addr));
    if (rv > 0) { die("bind()"); }

    /* s4: listen for incoming connections
    socket is actually created here */
    printf("Socket setup done ✅ \n");
    rv = listen(fd, SOMAXCONN);
    if (rv) { die("listen()"); }
    printf("Started listening to client... \n");

    // s5: accept a connection & close it
    while(true){
        struct sockaddr_in client_addr = {};
        socklen_t client_len = sizeof(client_addr);
        int connfd = accept(fd, (struct sockaddr *)&client_addr, &client_len);
        if (connfd < 0) {
            continue; // ERROR
        }
        printf("Accepted connection! \n");

        // s6: read/write data
        do_something(connfd);

        // Close connection
        close(connfd);
    }

    return 0;
}
