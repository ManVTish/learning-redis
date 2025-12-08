#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>

const size_t k_max_msg = 4096;

static void msg(const char *msg) {
    fprintf(stderr, "%s\n", msg);
}

static void die(const char * msg) {
    int err = errno;
    fprintf(stderr, "[%d] %s\n", err, msg);
    abort();
}

static int32_t read_full(int connfd, char *buff, size_t n) {
    while(n > 0) {
        ssize_t rv = read(connfd, buff, n);
        if (rv <= 0) {
            return -1;
        }

        assert(size_t(rv) <= n);
        buff += rv;
        n -= size_t(rv);
    }
    return 0;
}

static int32_t write_all(int connfd, char *buff, size_t n) {
    while(n > 0) {
        ssize_t rv = write(connfd, buff, n);
        if (rv <=0 ) {
            return -1;
        }

        assert(size_t(rv) <= n);
        buff += rv;
        n -= size_t(rv);
    }
    return 0;
}

static int32_t one_request(int connfd) {
    // read first 4 bytes
    printf("Request received!");
    char rbuf[4 + k_max_msg];
    errno = 0;
    int32_t err = read_full(connfd, rbuf, 4);
    if (err) {
        msg(errno == 0 ? "EOF" : "read() error");
        return err;
    }
    uint32_t len = 0;
    memcpy(&len, rbuf, 4);  // copy bytes to len
    if (len > k_max_msg) {
        msg("message too long");
        return -1;
    }
    printf("Request body received ✅ ");

    // read request body
    err = read_full(connfd, rbuf + 4, len); // &rbuf[4] is same as rbuf + 4
    if (err) {
        msg("read() error");
        return err;
    }
    printf("client says: %.*s\n", len, &rbuf[4]);

    // write response using same protocol
    const char reply[] = "Hello from server!";
    char wbuff[4 + sizeof(reply)];
    len = (uint32_t)strlen(reply);
    memcpy(wbuff, &len, 4);
    memcpy(wbuff+4, reply, len);
    return write_all(connfd, wbuff, 4 + len);
}

int main() {
    // Get socket handler
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        die("socket()");
    }

    // set socket options
    int val = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    // bind socket to address and port
    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(1234); // port
    addr.sin_addr.s_addr = htonl(0); // IP 0.0.0.0 wildcard address
    int rv = bind(fd, (const sockaddr *)&addr, sizeof(addr));
    if (rv) {
        die("bind()");
    }
    printf("Server setup done ✅ \n");

    // listen on the socket
    rv = listen(fd, SOMAXCONN);
    if (rv) {
        die("listen()");
    }
    printf("Server listening on port 1234... \n");

    // accept a connection & close the socket
    while(true) {
        struct sockaddr_in client_addr = {};
        socklen_t client_len = sizeof(client_addr);
        int connfd = accept(fd, (struct sockaddr *)&client_addr, &client_len);
        if (connfd < 0) {
            continue; // ERROR
        }
        printf("Connection accepted from client ✌️ \n");

        while(true) {
            // one connection per
            printf("Waiting for request... \n"); 
            int32_t err = one_request(connfd);
            if (err) {
                break;
            }
            printf("Request completed! \n"); 
        }

        // close connection
        close(fd);
    }

    return 0;
}
