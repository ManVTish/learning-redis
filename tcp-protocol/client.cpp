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

static int32_t query(int fd, const char *buff) {
    // message length validation
    uint32_t len = (uint32_t)strlen(buff);
    if (len > k_max_msg) {
        return -1;
    }

    // send request
    char wbuff[4 + k_max_msg];
    memcpy(wbuff, &len, 4);
    memcpy(wbuff + 4, buff, len);

    if(int err = write_all(fd, wbuff, 4 + len)) {
        msg("write() error");
        return err;
    }
    printf("Request sent ✅ ");

    // read response (first 4 bytes, then reply body)
    char rbuff[4 + k_max_msg + 1];
    errno = 0;
    int32_t err = read_full(fd, rbuff, 4);

    if (err) {
        msg(errno == 0 ? "EOF" : "read() error");
        return err;
    }
    memcpy(&len, rbuff, 4);
    if (len > k_max_msg) {
        msg("message too long");
        return -1;
    }

    err = read_full(fd, rbuff + 4, len);
    if (err) {
        msg("read() error");
        return  err;
    }
    printf("Received from server: %.*s\n", len, &rbuff[4]);

    return 0;
}

int main() {
    // Get socket handler
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        die("socket()");
    }

    // bind socket to address and port
    struct sockaddr_in srv_addr = {};
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = ntohs(1234);
    srv_addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK); // 127.0.0.1

    // connect to server
    int rv = connect(fd, (const struct sockaddr *)&srv_addr, sizeof(srv_addr));
    if (rv) {
        die("connect");
    }
    printf("Connected to server ✅ \n");

    // multiple requests
    printf("Sending requests initiated... \n");
    int32_t err = query(fd, "hello1");
    if(err) {
        goto L_DONE;
    }

    err = query(fd, "hello2");
    if(err) {
        goto L_DONE;
    }

    err = query(fd, "hello3");
    if (err) {
        goto L_DONE;
    }
    printf("All requests completed ✅ \n");

L_DONE:
    close(fd);
    return 0;
}
