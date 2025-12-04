#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#define MAX_NUMBERS 100

int main(int argc, char **argv) {
    int file = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (file == -1) {
        const char msg[] = "error: failed to open requested file\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    }

    char buf[4096];
    ssize_t bytes;
    float numbers[MAX_NUMBERS];

    while ((bytes = read(STDIN_FILENO, buf, sizeof(buf))) > 0) {
        if (bytes < 0) {
            const char msg[] = "error: failed to read from stdin\n";
            write(STDERR_FILENO, msg, sizeof(msg));
            exit(EXIT_FAILURE);
        }
        buf[bytes] = '\0';

        int count = 0;
        char *token = strtok(buf, " \t\n");
        
        while (token != NULL && count < MAX_NUMBERS) {
            int valid = 1;
            int dot_count = 0;
            for (int i = 0; token[i] != '\0'; i++) {
                if (!isdigit(token[i]) && token[i] != '.' && 
                    !(i == 0 && token[i] == '-')) {
                    valid = 0;
                    break;
                }
                if (token[i] == '.') dot_count++;
            }
            
            if (valid && dot_count <= 1) {
                numbers[count++] = atof(token);
            }
            token = strtok(NULL, " \t\n");
        }

        if (count < 2) {
            const char error_msg[] = "ERROR: Need at least 2 numbers\n";
            write(STDOUT_FILENO, error_msg, sizeof(error_msg) - 1);
            
            const char file_error[] = "Ошибка: необходимо минимум 2 числа\n";
            write(file, file_error, sizeof(file_error) - 1);
            continue;
        }

        float first = numbers[0];
        
        char calculation[512];
        int calc_len = snprintf(calculation, sizeof(calculation), 
                               "Вычисление: %.2f / ", first);
        write(file, calculation, calc_len);
        
        for (int i = 1; i < count; i++) {
            calc_len = snprintf(calculation, sizeof(calculation), 
                               "%.2f", numbers[i]);
            write(file, calculation, calc_len);
            
            if (i < count - 1) {
                const char divider[] = " / ";
                write(file, divider, sizeof(divider) - 1);
            }
        }
        const char newline[] = "\n";
        write(file, newline, sizeof(newline) - 1);

        float result = first;
        for (int i = 1; i < count; i++) {
            if (numbers[i] == 0.0f) {
                const char error_msg[] = "DIVISION_BY_ZERO\n";
                write(STDOUT_FILENO, error_msg, sizeof(error_msg) - 1);
                
                const char file_error[] = "ОШИБКА: деление на ноль!\n";
                write(file, file_error, sizeof(file_error) - 1);
                
                close(file);
                exit(EXIT_FAILURE);
            }
            result /= numbers[i];
        }
        char result_str[128];
        int result_len = snprintf(result_str, sizeof(result_str), 
                                 "Результат: %.6f\n\n", result);
        write(file, result_str, result_len);

        const char success_msg[] = "OK\n";
        write(STDOUT_FILENO, success_msg, sizeof(success_msg) - 1);
    }

    close(file);
    return 0;
}