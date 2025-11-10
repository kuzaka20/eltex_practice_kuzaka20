#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int charInInt(int x, int y, char* data){
    int num = 0;
    for(int i = x; i < y; i++){
        num *= 10;
        num += data[i] - '0';
    }
    return num;
}

int* parsingIP(char* str_ip){
    int count_num = 0, count_res = 0;
    int* ip = calloc(4, sizeof(int));
    for(size_t i = 0; i < strlen(str_ip) + 1; i++){
        if(str_ip[i] == '.' || i == (strlen(str_ip))){
            ip[count_res] = charInInt(i-count_num, i, str_ip);
            count_num = 0;
            count_res++;
            i++;
        }
        count_num++;
    }
    return ip;
}

void printBinary(char n) {
    for (int i = 7; i >= 0; i--) {
        printf("%d", (n >> i) & 1);
    }
    printf("\n");
}

char intToBin(int data){
    char bin_data = 0b00000000;
    char test = 0b00000001;
    int tmp[8] ={0,0,0,0,0,0,0,0};
    for(int i = 0; i < 8; i++){
        tmp[7-i] = data % 2;
        data = data/2;
    }
    for(int i = 0; i < 8; i++){
        if(tmp[i] == 1){
            test = test << (7 - i);
            bin_data = bin_data | test;
            test = 0b0000001;
        }
    }
    return bin_data;
}

int cheakBin(char bin_ip[4], char bin_net[4]){
    int flag = 0;
    for(int i = 0; i < 4;i++){
        if(bin_ip[i] != bin_net[i]){
            flag = 1;
            break;
        }
    }
    return flag;

}

void createNet(char bin_ip[4], char bin_mask[4], char net[4]){
    for(int i = 0; i < 4; i++){
        net[i] = bin_ip[i] & bin_mask[i];
    }
}

int checkRandom(char net[4], char bin_mask[4], int count){
    int result = 0;
    for(int i = 0; i < count; i++){
        char random_ip[4] = {rand()%256, rand()%256, rand()%256, rand()%256}; 
        char net_random_ip[4];
        createNet(random_ip, bin_mask, net_random_ip);
        if(cheakBin(net_random_ip, net) == 0){
            result++;
        }
    }
    return result;
}

int main(int argc, char *argv[]){
    if (argc != 4){
        printf("incorrect number of parameters\n");
        return 1;
    }
    int* ip = calloc(4, sizeof(int));
    int* mask = calloc(4, sizeof(int));
    ip = parsingIP(argv[1]);
    mask = parsingIP(argv[2]);
    int count_packages = charInInt(0, strlen(argv[3]), argv[3]);
    printf("%d.%d.%d.%d\n", ip[0], ip[1], ip[2], ip[3] );
    printf("%d.%d.%d.%d\n", mask[0], mask[1], mask[2], mask[3]);
    printf("%d\n", count_packages);
    char bin_ip[4] = {intToBin(ip[0]), intToBin(ip[1]), intToBin(ip[2]), intToBin(ip[3])};
    char bin_mask[4] = {intToBin(mask[0]), intToBin(mask[1]), intToBin(mask[2]), intToBin(mask[3])};
    char net[4];
    createNet(bin_ip, bin_mask, net);
    int result = checkRandom(net, bin_mask, count_packages);
    printf("%d\n", result);
    printf("%.2f", result/ (double)count_packages * 100);
    return 0; 
}