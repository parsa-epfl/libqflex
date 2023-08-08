#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <glib.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct MultiNodeCfg {
    int slave_idx;
    int socket_s2m; // Receive messages socket
    int socket_m2s; // Send messages socket
} MultiNodeCfg;

MultiNodeCfg config;

static int init_s2m_receiver(const char* socket_path, int slave_idx) {
    g_autoptr(GString) socket_name = g_string_new("");
    g_string_append_printf(socket_name, "%s/s2m_%i", socket_path, slave_idx);

    // Create a socket
    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd == -1) {
        fprintf(stderr, "Could not allocate Linux Socket for multi-node slave %i\n", slave_idx);
        return 1;
    }

    // Set up the receiver's address
    struct sockaddr_un receiver_addr;
    memset(&receiver_addr, 0, sizeof(receiver_addr));
    receiver_addr.sun_family = AF_UNIX;
    strcpy(receiver_addr.sun_path, socket_name->str); // Path to the receiver's socket
    printf("Socket path: %s\n", receiver_addr.sun_path);

    // Bind the socket to the address
    if (bind(sockfd, (struct sockaddr*)&receiver_addr, sizeof(receiver_addr)) == -1) {
        fprintf(stderr, "S[%i]:multinode:Could not bind socket for slave\n", slave_idx);
        return 1;
    }

    // Listen for incoming connections
    if (listen(sockfd, 10) == -1) {
        fprintf(stderr, "S[%i]:multinode:Could not listen socket for slave\n", slave_idx);
        return 1;
    }

    printf("S[%i]:multinode:Connected to sync server\n", slave_idx);

    config.socket_s2m = sockfd;
    return 0;
}

static int init_m2s_sender(const char* socket_path, int slave_idx) {
    g_autoptr(GString) socket_name = g_string_new("");
    g_string_append_printf(socket_name, "%s/m2s_%i", socket_path, slave_idx);

    // Create a socket
    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket");
        return 1;
    }

    // Set up the receiver's address
    struct sockaddr_un receiver_addr;
    memset(&receiver_addr, 0, sizeof(receiver_addr));
    receiver_addr.sun_family = AF_UNIX;
    strcpy(receiver_addr.sun_path, socket_name->str); // Path to the receiver's socket

    // Connect to the receiver
    int err = 0;
    while ((err = connect(sockfd, (struct sockaddr*)&receiver_addr, sizeof(receiver_addr)))) {
        switch (err) {
            case ECONNREFUSED:
                printf("S:[%i]: Connection failed, retrying in 2 seconds", slave_idx);
                sleep(2);
                break;
            default:
                fprintf(stderr, "S[%i]:multinode:Could not connect to master Sync Server, with errno: %i\n", slave_idx, errno);
                return 1;
        }
    }

    config.socket_m2s = sockfd;
    return 0;
}

int s2m_send_message(int sockfd, int slave_idx) {
    // Send data
    bool message = true;
    if (send(sockfd, &message, sizeof(message), 0) == -1) {
        perror("send");
        return 1;
    }

    return 0;
}

int m2s_receive_msg(int sockfd, int slave_idx) {
    uint64_t budget;

    printf("S[%i]:multinode:Waiting for a connection...\n", slave_idx);

    // Accept a connection
    struct sockaddr_un sender_addr;
    socklen_t sender_addr_len = sizeof(sender_addr);
    int sender_sockfd = accept(sockfd, (struct sockaddr*)&sender_addr, &sender_addr_len);
    if (sender_sockfd == -1) {
        fprintf(stderr, "S[%i]:multinode:Could not accept socket for slave\n", slave_idx);
        return 1;
    }

    // Receive data
    int bytes_received = recv(sender_sockfd, &budget, sizeof(budget), 0);
    if (bytes_received == -1) {
        perror("recv");
        return 1;
    }

    printf("Received: %lu\n", budget);
    close(sender_sockfd);
    return 0;
}

int quantum_slave_init(const char *socket_path, int slave_idx) {
    int ret = 0;
    config.slave_idx = slave_idx;
    if (!config.socket_s2m) {
        if ((ret = init_s2m_receiver(socket_path, slave_idx))) {
            return ret;
        }
    }
    if (!config.socket_m2s) {
        ret = init_m2s_sender(socket_path, slave_idx);
    }
    return ret;
}

int quantum_slave_close(void) {
    if (config.socket_s2m) close(config.socket_s2m);
    if (config.socket_m2s) close(config.socket_m2s);
    return 0;
}

int main(void) {
    const char *socket_path = "/tmp/qflex/";
    const int slave_idx = 0;
    quantum_slave_init(socket_path, slave_idx);
    m2s_receive_msg(config.socket_s2m, slave_idx);
    s2m_send_message(config.socket_m2s, slave_idx);
    quantum_slave_close();
    return 1;
}

