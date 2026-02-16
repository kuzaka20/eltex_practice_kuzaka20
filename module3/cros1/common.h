#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/in.h>
#include <time.h>
#include <errno.h>
#include <stdbool.h>
#include <pthread.h>

#define BUFFER_SIZE 1024
#define MAX_CLIENTS 100
#define CLIENT_TIMEOUT 60

typedef struct {
    struct in_addr ip;
    uint16_t port;
    int counter;
    time_t last_seen;
} client_info;

typedef struct {
    struct ip ip_hdr;
    struct udphdr udp_hdr;
    char payload[BUFFER_SIZE];
} packet_t;

unsigned short checksum(void *b, int len);

#endif
