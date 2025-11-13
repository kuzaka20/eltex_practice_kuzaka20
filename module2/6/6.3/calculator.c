#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <dirent.h>

#define MAX_FUNCTIONS 100
#define LIBRARY_DIR "./libs"

typedef struct {
    char name[50];
    char symbol;
    int (*function)(int, int);
    void* library_handle;
} operation_t;

operation_t operations[MAX_FUNCTIONS];
int operation_count = 0;

int charInInt(int x, int y, char* data) {
    int num = 0;
    for (int i = x; i < y; i++) {
        num *= 10;
        num += data[i] - '0';
    }
    printf("Parsed number: %d (positions %d-%d)\n", num, x, y);
    return num;
}

void shift(char *operators, int *arguments, int num, int result, int count_operators) {
    for (int i = num; i < count_operators - 1; i++) {
        operators[i] = operators[i + 1];
    }
    
    arguments[num] = result;
    for (int i = num + 1; i < count_operators; i++) {
        arguments[i] = arguments[i + 1];
    }
}

void count_arithmetic(char* operators, int* arguments, int num, int count_operators) {
    int result = 0;
    char current_operator = operators[num];
    int error_occurred = 0;
    
    for (int i = 0; i < operation_count; i++) {
        if (operations[i].symbol == current_operator) {
            if (current_operator == '/' && arguments[num + 1] == 0) {
                printf("Error: Division by zero! Operation: %d / %d\n", 
                       arguments[num], arguments[num + 1]);
                error_occurred = 1;
                result = 0;
            } else {
                result = operations[i].function(arguments[num], arguments[num + 1]);
                printf("Calculated: %d %c %d = %d\n", 
                       arguments[num], current_operator, arguments[num + 1], result);
            }
            break;
        }
    }
    
    if (!error_occurred) {
        shift(operators, arguments, num, result, count_operators);
    } else {
        printf("Calculation stopped due to error.\n");
        exit(1);
    }
}

void set_priority(int* priority_operator, int count_operators, char* operators) {
    for (int i = 0; i < count_operators; i++) {
        for (int j = 0; j < operation_count; j++) {
            if (operations[j].symbol == operators[i]) {
                if (strstr(operations[j].name, "mul") != NULL || 
                    strstr(operations[j].name, "div") != NULL) {
                    priority_operator[i] = 2;
                } else {
                    priority_operator[i] = 1;
                }
                break;
            }
        }
        printf("%d ", priority_operator[i]);
    }
    printf("\n");
}

void decrease_priority(int* priority_operator, int count, int num) {
    int* tmp_priority = calloc(count - 1, sizeof(int)); 
    for (int i = 0; i < num; i++) {
        tmp_priority[i] = priority_operator[i];
    }
    for (int i = num; i < count; i++) {
        tmp_priority[i] = priority_operator[i + 1];
    }
    
    for (int i = 0; i < count - 1; i++) {
        priority_operator[i] = tmp_priority[i];
    }
    free(tmp_priority);
}

void arithmetic(char* operators, int* arguments, int count_operators) {
    int* priority_operator = calloc(count_operators, sizeof(int));
    int flag_priority_two = 1, flag = 0;
    
    set_priority(priority_operator, count_operators, operators);
    
    while (count_operators != 0) {
        for (int i = 0; i < count_operators; i++) {
            if (flag_priority_two == 1) {
                if (priority_operator[i] == 2) {
                    count_arithmetic(operators, arguments, i, count_operators);
                    decrease_priority(priority_operator, count_operators, i);
                    count_operators--;
                    flag = 1;
                    break;
                }
            } else {
                count_arithmetic(operators, arguments, i, count_operators);
                decrease_priority(priority_operator, count_operators, i);
                count_operators--;
                break;
            }
        }
        
        printf("Arguments: ");
        for (int i = 0; i < count_operators + 1; i++) {
            printf("%d ", arguments[i]);
        }
        printf("\nOperators: ");
        for (int i = 0; i < count_operators; i++) {
            printf("%c ", operators[i]);
        }
        printf("\n");
        
        if (flag == 0) {
            flag_priority_two = 0;
        } else {
            flag = 0;
        }
    }
    printf("Final result: %d\n", arguments[0]);
    free(priority_operator);
}

void load_functions_from_libraries() {
    DIR *dir;
    struct dirent *entry;
    
    printf("Loading functions from directory: %s\n", LIBRARY_DIR);
    
    dir = opendir(LIBRARY_DIR);
    if (!dir) {
        printf("Error: Cannot open directory %s\n", LIBRARY_DIR);
        return;
    }
    
    while ((entry = readdir(dir)) != NULL) {
        if (strstr(entry->d_name, ".so") != NULL) {
            char lib_path[256];
            snprintf(lib_path, sizeof(lib_path), "%s/%s", LIBRARY_DIR, entry->d_name);
            
            void* library = dlopen(lib_path, RTLD_LAZY);
            if (!library) {
                printf("Error loading library %s: %s\n", lib_path, dlerror());
                continue;
            }
            
            char* (*get_symbol)(void) = dlsym(library, "get_operation_symbol");
            int (*func_ptr)(int, int) = dlsym(library, "execute");
            char* (*get_name)(void) = dlsym(library, "get_operation_name");
            
            if (get_symbol && func_ptr && get_name) {
                if (operation_count < MAX_FUNCTIONS) {
                    strncpy(operations[operation_count].name, get_name(), 
                           sizeof(operations[operation_count].name) - 1);
                    operations[operation_count].symbol = get_symbol()[0];
                    operations[operation_count].function = func_ptr;
                    operations[operation_count].library_handle = library;
                    
                    printf("Loaded function: %s (operator: '%c')\n", 
                           operations[operation_count].name, operations[operation_count].symbol);
                    operation_count++;
                } else {
                    printf("Error: Maximum function count reached\n");
                    dlclose(library);
                }
            } else {
                printf("Error: Invalid library format in %s\n", lib_path);
                dlclose(library);
            }
        }
    }
    
    closedir(dir);
}

void cleanup_libraries() {
    for (int i = 0; i < operation_count; i++) {
        if (operations[i].library_handle) {
            dlclose(operations[i].library_handle);
        }
    }
}

void parsingStr(char* data, int n) {
    int count_operators = 0, flag = 0, count_arguments = 0, count_num = 0, flag_num = 0, flag_error = 0;
    char* operators = calloc(1, sizeof(char));
    int* arguments = calloc(1, sizeof(int));
    char num[10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
    
    for (int i = 0; i < n; i++) {
        if (flag == 1) {
            int valid_operator = 0;
            for (int j = 0; j < operation_count; j++) {
                if (data[i] == operations[j].symbol) {
                    valid_operator = 1;
                    break;
                }
            }
            
            if (valid_operator) {
                flag = 0;
                count_operators++;
                operators = realloc(operators, sizeof(char) * count_operators);
                operators[count_operators - 1] = data[i];
            }
        } else if (flag == 0) {
            for (int j = 0; j < 10; j++) {
                if (data[i] == num[j]) {
                    count_num++;
                    flag_num = 1;
                    break;
                }
            }
            if (count_num != 0 && flag_num == 0) {
                count_arguments++;
                flag = 1;
                arguments = realloc(arguments, sizeof(int) * count_arguments);
                arguments[count_arguments - 1] = charInInt(i - count_num, i, data);
                i--;
                count_num = 0;
            }
            flag_num = 0;
        } else if (data[i] != '=') {
            printf("Incorrect input: unexpected character '%c'\n", data[i]);
            flag_error = 1;
            break;
        }
    }
    
    if (flag_error == 0) {
        printf("Arguments (%d): ", count_arguments);
        for (int i = 0; i < count_arguments; i++) {
            printf("%d ", arguments[i]);
        }
        printf("\nOperators (%d): ", count_operators);
        for (int i = 0; i < count_operators; i++) {
            printf("%c ", operators[i]);
        }
        printf("\n");
        
        if (count_arguments == count_operators + 1) {
            arithmetic(operators, arguments, count_operators);
        } else {
            printf("Error: Number of arguments and operators don't match\n");
        }
    }
    
    free(operators);
    free(arguments);
}

int main() {
    printf("=== Dynamic Calculator ===\n");
    
    load_functions_from_libraries();
    
    if (operation_count == 0) {
        printf("No functions loaded. Please check library directory.\n");
        return 1;
    }
    
    printf("\nAvailable operators: ");
    for (int i = 0; i < operation_count; i++) {
        printf("'%c' (%s) ", operations[i].symbol, operations[i].name);
    }
    printf("\n\n");
    
    int n = 0;
    char* calc_string = NULL;
    char simbol;
    
    printf("Enter expression (end with = ): ");
    
    while (scanf("%c", &simbol) == 1 && simbol != '\n') {
        calc_string = realloc(calc_string, sizeof(char) * (n + 1));
        calc_string[n] = simbol;
        n++;
    }
    
    if (n == 0 || calc_string[n - 1] != '=') {
        printf("Error: Expression must end with '='\n");
        free(calc_string);
        cleanup_libraries();
        return 1;
    }
    
    printf("Expression: ");
    for (int i = 0; i < n; i++) {
        printf("%c", calc_string[i]);
    }
    printf("\n");
    
    parsingStr(calc_string, n);
    
    free(calc_string);
    cleanup_libraries();
    
    return 0;
}