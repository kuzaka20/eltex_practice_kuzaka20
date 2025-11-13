#include <stdio.h>

__attribute__((visibility("default")))
int execute(int a, int b) {
    return a * b;
}

__attribute__((visibility("default")))
char* get_operation_symbol() {
    return "*";
}

__attribute__((visibility("default")))
char* get_operation_name() {
    return "multiplication";
}