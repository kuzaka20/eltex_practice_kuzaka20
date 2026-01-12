#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/if_ether.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <net/if.h>

#define BUFFER_SIZE 65536
#define LOG_FILE "packet_dump.bin"
#define TEXT_LOG_FILE "packet_analysis.txt"

int running = 1;

struct pseudo_header {
    uint32_t source_address;
    uint32_t dest_address;
    uint8_t placeholder;
    uint8_t protocol;
    uint16_t udp_length;
};

char* repeat_char(char c, int count) {
    static char buffer[256];
    memset(buffer, 0, sizeof(buffer));
    
    if (count > 255) count = 255;
    for (int i = 0; i < count; i++) {
        buffer[i] = c;
    }
    buffer[count] = '\0';
    return buffer;
}

void handle_signal(int sig) {
    (void)sig;
    printf("\nПолучен сигнал завершения. Завершение работы...\n");
    running = 0;
}

void print_packet_data(unsigned char* data, int size, FILE* text_log) {
    int i, j;
    
    for (i = 0; i < size; i += 16) {
        fprintf(text_log, "%04x: ", i);
        
        for (j = 0; j < 16; j++) {
            if (i + j < size) {
                fprintf(text_log, "%02x ", data[i + j]);
            } else {
                fprintf(text_log, "   ");
            }
            
            if (j == 7) fprintf(text_log, " ");
        }
        
        fprintf(text_log, " ");
        
        for (j = 0; j < 16; j++) {
            if (i + j < size) {
                unsigned char c = data[i + j];
                if (c >= 32 && c <= 126) {
                    fprintf(text_log, "%c", c);
                } else {
                    fprintf(text_log, ".");
                }
            } else {
                fprintf(text_log, " ");
            }
        }
        fprintf(text_log, "\n");
    }
}

unsigned short checksum(unsigned short *ptr, int nbytes) {
    register long sum;
    unsigned short oddbyte;
    register short answer;

    sum = 0;
    while (nbytes > 1) {
        sum += *ptr++;
        nbytes -= 2;
    }

    if (nbytes == 1) {
        oddbyte = 0;
        *((unsigned char*)&oddbyte) = *(unsigned char*)ptr;
        sum += oddbyte;
    }

    sum = (sum >> 16) + (sum & 0xffff);
    sum = sum + (sum >> 16);
    answer = (short)~sum;
    
    return answer;
}

void process_udp_packet(unsigned char* buffer, int size, FILE* bin_log, FILE* text_log) {
    struct iphdr *iph = (struct iphdr*)(buffer + sizeof(struct ethhdr));
    unsigned short iphdrlen = iph->ihl * 4;
    
    struct udphdr *udph = (struct udphdr*)(buffer + iphdrlen + sizeof(struct ethhdr));
    
    struct in_addr src_addr, dst_addr;
    src_addr.s_addr = iph->saddr;
    dst_addr.s_addr = iph->daddr;
    
    fprintf(text_log, "\n=== IP Пакет ===\n");
    fprintf(text_log, "Версия IP: %d\n", iph->version);
    fprintf(text_log, "Длина IP заголовка: %d байт\n", iph->ihl * 4);
    fprintf(text_log, "Протокол: %d (UDP=%d)\n", iph->protocol, IPPROTO_UDP);
    fprintf(text_log, "Источник: %s\n", inet_ntoa(src_addr));
    fprintf(text_log, "Назначение: %s\n", inet_ntoa(dst_addr));
    fprintf(text_log, "Длина пакета: %d байт\n", ntohs(iph->tot_len));
    
    fprintf(text_log, "\n=== UDP Пакет ===\n");
    fprintf(text_log, "Порт источника: %d\n", ntohs(udph->source));
    fprintf(text_log, "Порт назначения: %d\n", ntohs(udph->dest));
    fprintf(text_log, "Длина UDP: %d байт\n", ntohs(udph->len));
    fprintf(text_log, "Контрольная сумма: 0x%04x\n", ntohs(udph->check));
    
    int udp_header_len = sizeof(struct udphdr);
    int data_len = ntohs(udph->len) - udp_header_len;
    
    if (data_len > 0) {
        unsigned char* udp_data = (unsigned char*)udph + udp_header_len;
        
        fprintf(text_log, "\n=== Данные UDP (%d байт) ===\n", data_len);
        
        if (ntohs(udph->dest) == 8888) {
            fprintf(text_log, "ВНИМАНИЕ: Это пакет для чат-сервера (порт 8888)\n");
            
            fprintf(text_log, "Текст сообщения: ");
            for (int i = 0; i < data_len; i++) {
                if (udp_data[i] >= 32 && udp_data[i] <= 126) {
                    fprintf(text_log, "%c", udp_data[i]);
                } else if (udp_data[i] == 0) {
                    fprintf(text_log, "\\0");
                } else {
                    fprintf(text_log, ".");
                }
            }
            fprintf(text_log, "\n");
            
            if (data_len >= 5 && strncmp((char*)udp_data, "/quit", 5) == 0) {
                fprintf(text_log, "КОМАНДА: Запрос на выход из чата\n");
            }
            if (data_len >= 5 && strncmp((char*)udp_data, "/join", 5) == 0) {
                fprintf(text_log, "КОМАНДА: Запрос на присоединение к чату\n");
            }
        }
        
        fprintf(text_log, "\nHex дамп данных:\n");
        print_packet_data(udp_data, data_len, text_log);
        
        struct pseudo_header psh;
        psh.source_address = iph->saddr;
        psh.dest_address = iph->daddr;
        psh.placeholder = 0;
        psh.protocol = IPPROTO_UDP;
        psh.udp_length = udph->len;
        
        char *checksum_buf = (char*)malloc(sizeof(struct pseudo_header) + ntohs(udph->len));
        if (checksum_buf) {
            memcpy(checksum_buf, &psh, sizeof(struct pseudo_header));
            memcpy(checksum_buf + sizeof(struct pseudo_header), udph, ntohs(udph->len));
            
            unsigned short calculated_check = checksum((unsigned short*)checksum_buf, 
                                                       sizeof(struct pseudo_header) + ntohs(udph->len));
            free(checksum_buf);
            
            if (calculated_check == 0) {
                fprintf(text_log, "Контрольная сумма UDP: ВЕРНА\n");
            } else {
                fprintf(text_log, "Контрольная сумма UDP: НЕВЕРНА (рассчитано: 0x%04x)\n", calculated_check);
            }
        }
    } else {
        fprintf(text_log, "Нет данных в UDP пакете\n");
    }
    
    fprintf(text_log, "\n%s\n", repeat_char('=', 60));
    fflush(text_log);
    
    if (bin_log) {
        fwrite(buffer, 1, size, bin_log);
        fflush(bin_log);
    }
}

int main() {
    int raw_socket;
    struct sockaddr saddr;
    socklen_t saddr_size = sizeof(saddr);
    unsigned char *buffer = (unsigned char*)malloc(BUFFER_SIZE);
    FILE *bin_log = NULL, *text_log = NULL;
    time_t rawtime;
    struct tm *timeinfo;
    char timestamp[80];
    
    if (!buffer) {
        fprintf(stderr, "Ошибка выделения памяти\n");
        return 1;
    }
    
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    
    bin_log = fopen(LOG_FILE, "wb");
    if (!bin_log) {
        perror("Ошибка открытия бинарного файла лога");
        free(buffer);
        return 1;
    }
    
    text_log = fopen(TEXT_LOG_FILE, "w");
    if (!text_log) {
        perror("Ошибка открытия текстового файла лога");
        fclose(bin_log);
        free(buffer);
        return 1;
    }
    
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);
    
    fprintf(text_log, "=== Начало захвата пакетов ===\n");
    fprintf(text_log, "Время начала: %s\n", timestamp);
    fprintf(text_log, "Логирование в файлы: %s (бинарный) и %s (текстовый)\n", LOG_FILE, TEXT_LOG_FILE);
    fprintf(text_log, "Ожидание UDP пакетов на порту 8888...\n");
    fprintf(text_log, "%s\n\n", repeat_char('=', 60));
    
    printf("=== UDP Packet Sniffer ===\n");
    printf("Сниффер запущен и ожидает пакеты...\n");
    printf("Порт для мониторинга: 8888 (UDP чат)\n");
    printf("Логирование в файлы:\n");
    printf("  - %s (бинарный дамп)\n", LOG_FILE);
    printf("  - %s (текстовый анализ)\n", TEXT_LOG_FILE);
    printf("Для остановки нажмите Ctrl+C\n\n");
    
    raw_socket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (raw_socket < 0) {
        perror("Ошибка создания RAW-сокета");
        fclose(bin_log);
        fclose(text_log);
        free(buffer);
        return 1;
    }
    
    int optval = 1;
    if (setsockopt(raw_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        perror("Ошибка setsockopt");
    }
    
    int packet_count = 0;
    
    while (running) {
        int packet_size = recvfrom(raw_socket, buffer, BUFFER_SIZE, 0, &saddr, &saddr_size);
        
        if (packet_size < 0) {
            if (!running) break;
            continue;
        }
        
        packet_count++;
        
        time(&rawtime);
        timeinfo = localtime(&rawtime);
        strftime(timestamp, sizeof(timestamp), "%H:%M:%S", timeinfo);
        
        printf("[%s] Пакет #%d: %d байт\n", timestamp, packet_count, packet_size);
        
        fprintf(text_log, "\n[%s] Пакет #%d: %d байт\n", timestamp, packet_count, packet_size);
        
        struct ethhdr *eth = (struct ethhdr*)buffer;
        
        if (ntohs(eth->h_proto) == ETH_P_IP) {
            struct iphdr *iph = (struct iphdr*)(buffer + sizeof(struct ethhdr));
            
            if (iph->protocol == IPPROTO_UDP) {
                struct in_addr src_ip;
                src_ip.s_addr = iph->saddr;
                printf("  -> UDP пакет от %s\n", inet_ntoa(src_ip));
                
                process_udp_packet(buffer, packet_size, bin_log, text_log);
                
                struct udphdr *udph = (struct udphdr*)(buffer + (iph->ihl * 4) + sizeof(struct ethhdr));
                if (ntohs(udph->dest) == 8888) {
                    printf("  -> НАПРАВЛЕН НА ЧАТ-СЕРВЕР (порт 8888)\n");
                    
                    int data_len = ntohs(udph->len) - sizeof(struct udphdr);
                    if (data_len > 0) {
                        unsigned char* data = (unsigned char*)udph + sizeof(struct udphdr);
                        printf("  -> Сообщение: ");
                        int max_display = (data_len < 50) ? data_len : 50;
                        for (int i = 0; i < max_display; i++) {
                            if (data[i] >= 32 && data[i] <= 126) {
                                putchar(data[i]);
                            } else if (data[i] == 0) {
                                printf("\\0");
                            } else {
                                putchar('.');
                            }
                        }
                        if (data_len >= 50) printf("...");
                        printf("\n");
                    }
                }
            } else if (iph->protocol == IPPROTO_TCP) {
                fprintf(text_log, "TCP пакет (не обрабатывается)\n");
            } else if (iph->protocol == IPPROTO_ICMP) {
                fprintf(text_log, "ICMP пакет (не обрабатывается)\n");
            } else {
                fprintf(text_log, "Другой IP протокол: %d\n", iph->protocol);
            }
        } else if (ntohs(eth->h_proto) == ETH_P_ARP) {
            fprintf(text_log, "ARP пакет (не обрабатывается)\n");
        } else {
            fprintf(text_log, "Неизвестный тип пакета: 0x%04x\n", ntohs(eth->h_proto));
        }
        
        fflush(text_log);
    }
    
    printf("\nЗавершение работы сниффера...\n");
    printf("Всего перехвачено пакетов: %d\n", packet_count);
    printf("Данные сохранены в файлах:\n");
    printf("  - %s (бинарный дамп)\n", LOG_FILE);
    printf("  - %s (текстовый анализ)\n", TEXT_LOG_FILE);
    
    fprintf(text_log, "\n%s\n", repeat_char('=', 60));
    fprintf(text_log, "=== Завершение захвата пакетов ===\n");
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);
    fprintf(text_log, "Время окончания: %s\n", timestamp);
    fprintf(text_log, "Всего пакетов: %d\n", packet_count);
    
    close(raw_socket);
    if (bin_log) fclose(bin_log);
    if (text_log) fclose(text_log);
    free(buffer);
    
    printf("\nДля анализа бинарного дампа используйте команды:\n");
    printf("  hexdump -C %s  # Просмотр hex дампа\n", LOG_FILE);
    printf("  strings %s     # Поиск текстовых строк в дампе\n", LOG_FILE);
    printf("  cat %s         # Просмотр текстового анализа\n", TEXT_LOG_FILE);
    
    return 0;
}