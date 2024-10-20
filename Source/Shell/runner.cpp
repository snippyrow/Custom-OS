#include "shell.h"

void shell_div() {
    int num0 = str_int64(shell_argtable[1]);
    int num1 = str_int64(shell_argtable[2]);
    char* result = int64_str(num0/num1);
    char* print = strcat("\n--> ",result);
    shell_tty_print(print);

    free(*result, strlen(result) + 2);
    free(*print, strlen(print) + 2);
}