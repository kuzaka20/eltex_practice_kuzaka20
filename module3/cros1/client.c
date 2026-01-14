#include "common.h"

static int running = 1;
static int raw_sock;
static struct sockaddr_in server_addr;

uint16_t get_unique_port() {
    static uint16_t port = 20001;
    return htons(port++);
}

void signal_handler(int sig) {
    (void)sig;
    running = 0;
}

void send_message(const char *msg, uint16_t src_port) {
    packet_t packet;
    memset(&packet, 0, sizeof(packet));

    struct sockaddr_in local;
    socklen_t len = sizeof(local);
    getsockname(raw_sock, (struct sockaddr *)&local, &len);

    packet.ip_hdr.ip_v = 4;
    packet.ip_hdr.ip_hl = 5;
    packet.ip_hdr.ip_ttl = 64;
    packet.ip_hdr.ip_p = IPPROTO_UDP;
    packet.ip_hdr.ip_src = local.sin_addr;
    packet.ip_hdr.ip_dst = server_addr.sin_addr;
    packet.ip_hdr.ip_len = htons(sizeof(struct ip) + sizeof(struct udphdr) + strlen(msg));
    packet.ip_hdr.ip_sum = checksum(&packet.ip_hdr, sizeof(struct ip));

    packet.udp_hdr.source = src_port;
    packet.udp_hdr.dest = server_addr.sin_port;
    packet.udp_hdr.len = htons(sizeof(struct udphdr) + strlen(msg));
    packet.udp_hdr.check = 0;

    strcpy(packet.payload, msg);

    sendto(raw_sock, &packet,
           sizeof(struct ip) + sizeof(struct udphdr) + strlen(msg),
           0, (struct sockaddr *)&server_addr, sizeof(server_addr));
}

void *receiver(void *arg) {
    (void)arg;
    char buffer[BUFFER_SIZE];

    while (running) {
        struct sockaddr_in addr;
        socklen_t len = sizeof(addr);
        int r = recvfrom(raw_sock, buffer, BUFFER_SIZE, 0,
                         (struct sockaddr *)&addr, &len);
        if (r <= 0) continue;

        struct ip *ip_hdr = (struct ip *)buffer;
        if (ip_hdr->ip_p != IPPROTO_UDP) continue;

        int ip_len = ip_hdr->ip_hl * 4;
        struct udphdr *udp_hdr = (struct udphdr *)(buffer + ip_len);

        if (ip_hdr->ip_src.s_addr != server_addr.sin_addr.s_addr) continue;
        if (udp_hdr->source != server_addr.sin_port) continue;

        char *payload = buffer + ip_len + sizeof(struct udphdr);
        int payload_len = r - ip_len - sizeof(struct udphdr);
        payload[payload_len] = '\0';

        printf("Ответ сервера: %s\n", payload);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Использование: %s <IP сервера> <порт>\n", argv[0]);
        return 1;
    }

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    raw_sock = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    if (raw_sock < 0) {
        perror("socket");
        return 1;
    }

    int one = 1;
    setsockopt(raw_sock, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one));

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(raw_sock);
        return 1;
    }

    printf("Клиент подключен к серверу %s:%s\n", argv[1], argv[2]);

    pthread_t tid;
    pthread_create(&tid, NULL, receiver, NULL);

    uint16_t my_port = get_unique_port();

    char buf[BUFFER_SIZE];
    while (running && fgets(buf, BUFFER_SIZE, stdin)) {
        buf[strcspn(buf, "\n")] = 0;
        if (*buf) send_message(buf, my_port);
    }

    send_message("CLOSE_CONNECTION", my_port);

    running = 0;
    pthread_join(tid, NULL);
    close(raw_sock);
    return 0;
}

unsigned short checksum(void *b, int len) {
    unsigned short *buf = b;
    unsigned int sum = 0;
    for (; len > 1; len -= 2) sum += *buf++;
    if (len) sum += *(unsigned char *)buf;
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    return ~sum;
}
