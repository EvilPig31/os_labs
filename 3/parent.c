#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>

#define SHM_SIZE 4096

static char CHILD_PROGRAM_NAME[] = "child";

int main(int argc, char **argv) {
    if (argc == 1) {
        char msg[1024];
        uint32_t len = snprintf(msg, sizeof(msg), "usage: %s filename\n", argv[0]);
        write(STDERR_FILENO, msg, len);
        exit(EXIT_SUCCESS);
    }

    char progpath[1024];
    ssize_t len = readlink("/proc/self/exe", progpath, sizeof(progpath) - 1);
    if (len == -1) {
        const char msg[] = "error: failed to read full program path\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }
    progpath[len] = '\0';
    char *last_slash = strrchr(progpath, '/');
    if (last_slash) *last_slash = '\0';

    pid_t pid = getpid();
    char shm_name[64], sem_parent_name[64], sem_child_name[64];
    snprintf(shm_name, sizeof(shm_name), "/shm_%d", pid);
    snprintf(sem_parent_name, sizeof(sem_parent_name), "/sem_parent_%d", pid);
    snprintf(sem_child_name, sizeof(sem_child_name), "/sem_child_%d", pid);

    int shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0600);
    if (shm_fd == -1) {
        const char msg[] = "error: shm_open failed\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }
    if (ftruncate(shm_fd, SHM_SIZE) == -1) {
        const char msg[] = "error: ftruncate failed\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }
    char *shm_ptr = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        const char msg[] = "error: mmap failed\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }

    sem_t *sem_parent = sem_open(sem_parent_name, O_CREAT, 0600, 0);
    sem_t *sem_child = sem_open(sem_child_name, O_CREAT, 0600, 0);
    if (sem_parent == SEM_FAILED || sem_child == SEM_FAILED) {
        const char msg[] = "error: sem_open failed\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }

    pid_t child = fork();
    switch (child) {
    case -1: {
        const char msg[] = "error: failed to spawn new process\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    } break;

    case 0: {
        char child_path[2048];
        snprintf(child_path, sizeof(child_path), "%s/%s", progpath, CHILD_PROGRAM_NAME);
        
        char *args[] = {
            CHILD_PROGRAM_NAME,
            argv[1],
            shm_name,
            sem_parent_name,
            sem_child_name,
            NULL
        };
        
        execv(child_path, args);
        
        const char msg[] = "error: failed to exec into new executable image\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    } break;

    default: {
        char msg[128];
        int32_t length = snprintf(msg, sizeof(msg),
            "%d: Parent process, child PID: %d\n", pid, child);
        write(STDOUT_FILENO, msg, length);

        const char prompt1[] = "Enter command type: num num num ...\n";
        write(STDOUT_FILENO, prompt1, sizeof(prompt1) - 1);
        const char prompt2[] = "Print 'exit' to out\n";
        write(STDOUT_FILENO, prompt2, sizeof(prompt2) - 1);

        char buf[4096];
        ssize_t bytes;

        while ((bytes = read(STDIN_FILENO, buf, sizeof(buf))) > 0) {
            if (bytes >= 4 && strncmp(buf, "exit", 4) == 0) {
                break;
            }

            size_t copy_size = (bytes < SHM_SIZE - 1) ? bytes : SHM_SIZE - 1;
            memcpy(shm_ptr, buf, copy_size);
            shm_ptr[copy_size] = '\0';
            
            sem_post(sem_child);
            sem_wait(sem_parent);

            if (strstr(shm_ptr, "DIVISION_BY_ZERO") != NULL) {
                const char error_msg[] = "Zero division found!\n";
                write(STDOUT_FILENO, error_msg, sizeof(error_msg) - 1);
                break;
            } else if (strstr(shm_ptr, "OK") != NULL) {
            } else if (strlen(shm_ptr) == 0) {
                const char child_exit_msg[] = "Child process ended.\n";
                write(STDOUT_FILENO, child_exit_msg, sizeof(child_exit_msg) - 1);
                break;
            }
        }

        snprintf(shm_ptr, SHM_SIZE, "exit");
        sem_post(sem_child);

        wait(NULL);
        const char parent_exit_msg[] = "Child process ended.\n";
        write(STDOUT_FILENO, parent_exit_msg, sizeof(parent_exit_msg) - 1);

        munmap(shm_ptr, SHM_SIZE);
        close(shm_fd);
        shm_unlink(shm_name);
        sem_close(sem_parent);
        sem_close(sem_child);
        sem_unlink(sem_parent_name);
        sem_unlink(sem_child_name);
    } break;
    }

    return 0;
}