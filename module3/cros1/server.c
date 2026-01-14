#include "common.h"

static client_info clients[MAX_CLIENTS];
static int server_running = 1;
static pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void signal_handler(int sig) {
    printf("\nПолучен сигнал %d, завершение работы...\n", sig);
    server_running = 0;
}

int find_or_create_client(struct in_addr ip, uint16_t port) {
    time_t now = time(NULL);
    int free_slot = -1;

    pthread_mutex_lock(&clients_mutex);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].ip.s_addr == ip.s_addr && clients[i].port == port) {
            clients[i].last_seen = now;
            pthread_mutex_unlock(&clients_mutex);
            return i;
        }
        if (free_slot == -1 && clients[i].ip.s_addr == 0) {
            free_slot = i;
        }
    }

    if (free_slot != -1) {
        clients[free_slot].ip = ip;
        clients[free_slot].port = port;
        clients[free_slot].counter = 0;
        clients[free_slot].last_seen = now;
        pthread_mutex_unlock(&clients_mutex);
        return free_slot;
    }

    pthread_mutex_unlock(&clients_mutex);
    return -1;
}

void cleanup_old_clients() {
    time_t now = time(NULL);
    pthread_mutex_lock(&clients_mutex);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].ip.s_addr != 0 &&
            (now - clients[i].last_seen) > CLIENT_TIMEOUT) {
            printf("Удаление устаревшего клиента: %s:%d\n",
                   inet_ntoa(clients[i].ip), ntohs(clients[i].port));
            memset(&clients[i], 0, sizeof(client_info));
        }
    }

    pthread_mutex_unlock(&clients_mutex);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Использование: %s <порт>\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);
    int raw_sock;
    char buffer[BUFFER_SIZE];

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    raw_sock = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    if (raw_sock < 0) {
        perror("socket");
        return 1;
    }

    int one = 1;
    setsockopt(raw_sock, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one));

    memset(clients, 0, sizeof(clients));

    printf("Сервер запущен на порту %d\n", port);

    while (server_running) {
        struct sockaddr_in addr;
        socklen_t addr_len = sizeof(addr);

        int received = recvfrom(raw_sock, buffer, BUFFER_SIZE, 0,
                                (struct sockaddr *)&addr, &addr_len);
        if (received <= 0)
            continue;

        struct ip *ip_hdr = (struct ip *)buffer;
        if (ip_hdr->ip_p != IPPROTO_UDP)
            continue;

        int ip_len = ip_hdr->ip_hl * 4;
        struct udphdr *udp_hdr = (struct udphdr *)(buffer + ip_len);

        if (ntohs(udp_hdr->dest) != port)
            continue;

        char *payload = buffer + ip_len + sizeof(struct udphdr);
        int payload_len = received - ip_len - sizeof(struct udphdr);
        if (payload_len >= BUFFER_SIZE) payload_len = BUFFER_SIZE - 1;
        payload[payload_len] = '\0';

        struct in_addr client_ip = ip_hdr->ip_src;
        uint16_t client_port = udp_hdr->source;

        if (strcmp(payload, "CLOSE_CONNECTION") == 0) {
            pthread_mutex_lock(&clients_mutex);
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i].ip.s_addr == client_ip.s_addr &&
                    clients[i].port == client_port) {
                    memset(&clients[i], 0, sizeof(client_info));
                    printf("Клиент %s:%d отключился\n",
                           inet_ntoa(client_ip), ntohs(client_port));
                    break;
                }
            }
            pthread_mutex_unlock(&clients_mutex);
            continue;
        }

        int idx = find_or_create_client(client_ip, client_port);
        if (idx < 0) {
            printf("Достигнут лимит клиентов\n");
            continue;
        }

        pthread_mutex_lock(&clients_mutex);
        clients[idx].counter++;
        int count = clients[idx].counter;
        pthread_mutex_unlock(&clients_mutex);

        printf("Сообщение от %s:%d: %s (count=%d)\n",
               inet_ntoa(client_ip), ntohs(client_port), payload, count);

        char response[BUFFER_SIZE];
        snprintf(response, sizeof(response), "%s %d", payload, count);

        packet_t packet;
        memset(&packet, 0, sizeof(packet));
        packet.ip_hdr.ip_v = 4;
        packet.ip_hdr.ip_hl = 5;
        packet.ip_hdr.ip_ttl = 64;
        packet.ip_hdr.ip_p = IPPROTO_UDP;
        packet.ip_hdr.ip_src = ip_hdr->ip_dst;
        packet.ip_hdr.ip_dst = ip_hdr->ip_src;
        packet.ip_hdr.ip_len = htons(sizeof(struct ip) + sizeof(struct udphdr) + strlen(response));
        packet.ip_hdr.ip_sum = checksum(&packet.ip_hdr, sizeof(struct ip));

        packet.udp_hdr.source = udp_hdr->dest;
        packet.udp_hdr.dest = udp_hdr->source;
        packet.udp_hdr.len = htons(sizeof(struct udphdr) + strlen(response));

        strcpy(packet.payload, response);

        struct sockaddr_in dst = {
            .sin_family = AF_INET,
            .sin_addr = client_ip,
            .sin_port = udp_hdr->source
        };

        sendto(raw_sock, &packet,
               sizeof(struct ip) + sizeof(struct udphdr) + strlen(response),
               0, (struct sockaddr *)&dst, sizeof(dst));

        if (rand() % 100 == 0) cleanup_old_clients();
    }

    close(raw_sock);
    printf("Сервер завершил работу\n");
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