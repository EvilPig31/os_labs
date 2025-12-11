#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <semaphore.h>

#define MAX_NUMBERS 100
#define SHM_SIZE 4096

int main(int argc, char **argv) {
    if (argc < 5) {
        char msg[256];
        uint32_t len = snprintf(msg, sizeof(msg), 
            "usage: %s filename shm_name sem_parent_name sem_child_name\n", argv[0]);
        write(STDERR_FILENO, msg, len);
        exit(EXIT_FAILURE);
    }

    int file = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (file == -1) {
        const char msg[] = "error: failed to open requested file\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }

    int shm_fd = shm_open(argv[2], O_RDWR, 0);
    if (shm_fd == -1) {
        const char msg[] = "error: shm_open failed\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }
    char *shm_ptr = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        const char msg[] = "error: mmap failed\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }

    sem_t *sem_parent = sem_open(argv[3], 0);
    sem_t *sem_child = sem_open(argv[4], 0);
    if (sem_parent == SEM_FAILED || sem_child == SEM_FAILED) {
        const char msg[] = "error: sem_open failed\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }

    float numbers[MAX_NUMBERS];
    while (1) {
        sem_wait(sem_child);

        if (strcmp(shm_ptr, "exit") == 0) {
            break;
        }

        char local_buf[SHM_SIZE];
        strncpy(local_buf, shm_ptr, sizeof(local_buf) - 1);
        local_buf[sizeof(local_buf) - 1] = '\0';

        int count = 0;
        char *token = strtok(local_buf, " \t\n");
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
            const char file_error[] = "Error: 2 numbers needed\n";
            write(file, file_error, sizeof(file_error) - 1);
            snprintf(shm_ptr, SHM_SIZE, "ERROR");
            sem_post(sem_parent);
            continue;
        }

        float first = numbers[0];
        char calculation[512];
        int calc_len = snprintf(calculation, sizeof(calculation), 
                               "Calculation: %.2f / ", first);
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
                const char file_error[] = "Error: zero division!\n";
                write(file, file_error, sizeof(file_error) - 1);
                snprintf(shm_ptr, SHM_SIZE, "DIVISION_BY_ZERO");
                sem_post(sem_parent);
                close(file);
                exit(EXIT_FAILURE);
            }
            result /= numbers[i];
        }
        
        char result_str[128];
        int result_len = snprintf(result_str, sizeof(result_str), 
                                 "Result: %.6f\n\n", result);
        write(file, result_str, result_len);

        snprintf(shm_ptr, SHM_SIZE, "OK");
        sem_post(sem_parent);
    }

    close(file);
    munmap(shm_ptr, SHM_SIZE);
    close(shm_fd);
    sem_close(sem_parent);
    sem_close(sem_child);
    
    return 0;
}