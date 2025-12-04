#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static char CHILD_PROGRAM_NAME[] = "child";

int main(int argc, char **argv) {
	if (argc == 1) {
		char msg[1024];
		uint32_t len = snprintf(msg, sizeof(msg) - 1, "usage: %s filename\n", argv[0]);
		write(STDERR_FILENO, msg, len);
		exit(EXIT_SUCCESS);
	}
    char progpath[1024];
    {
        ssize_t len = readlink("/proc/self/exe", progpath, sizeof(progpath) - 1);
        if (len == -1) {
            const char msg[] = "error: failed to read full program path\n";
            write(STDERR_FILENO, msg, sizeof(msg));
            exit(EXIT_FAILURE);
        }

        while (progpath[len] != '/')
            --len;

        progpath[len] = '\0';
    }

    int parent_to_child[2];
    if (pipe(parent_to_child) == -1) {
        const char msg[] = "error: failed to create pipe\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    }

    int child_to_parent[2];
    if (pipe(child_to_parent) == -1) {
        const char msg[] = "error: failed to create pipe\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    }

    const pid_t child = fork();

    switch (child) {
    case -1: {
        const char msg[] = "error: failed to spawn new process\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    } break;

    case 0: {
        close(parent_to_child[1]);
        close(child_to_parent[0]);

        dup2(parent_to_child[0], STDIN_FILENO);
        close(parent_to_child[0]);

        dup2(child_to_parent[1], STDOUT_FILENO);
        close(child_to_parent[1]);

        {
            char path[1024];
            size_t path_len = snprintf(path, sizeof(path) - 1, "%s/%s", progpath, CHILD_PROGRAM_NAME);
            if (path_len >= sizeof(path)) {
                const char msg[] = "error: path too long\n";
                write(STDERR_FILENO, msg, sizeof(msg));
                exit(EXIT_FAILURE);
            }

            char *const args[] = {CHILD_PROGRAM_NAME, argv[1], NULL};

            int32_t status = execv(path, args);

            if (status == -1) {
                const char msg[] = "error: failed to exec into new executable image\n";
                write(STDERR_FILENO, msg, sizeof(msg));
                exit(EXIT_FAILURE);
            }
        }
    } break;

    default: {
        close(parent_to_child[0]);
        close(child_to_parent[1]);

        {
            pid_t pid = getpid();
            char msg[128];
            const int32_t length = snprintf(msg, sizeof(msg),
                "%d: Родительский процесс, дочерний PID: %d\n", pid, child);
            write(STDOUT_FILENO, msg, length);
        }

        printf("Введите команды в формате: число число число ...\n");
        printf("Для выхода введите 'exit'\n");

        char buf[4096];
        ssize_t bytes;

        while (bytes = read(STDIN_FILENO, buf, sizeof(buf))) {
            if (bytes < 0) {
                const char msg[] = "error: failed to read from stdin\n";
                write(STDERR_FILENO, msg, sizeof(msg));
                exit(EXIT_FAILURE);
            }
            if (bytes >= 4 && strncmp(buf, "exit", 4) == 0) {
                break;
            }
            write(parent_to_child[1], buf, bytes);

            bytes = read(child_to_parent[0], buf, sizeof(buf));
            if (bytes > 0) {
                if (strstr(buf, "DIVISION_BY_ZERO") != NULL) {
                    printf("Обнаружено деление на ноль! Завершение работы.\n");
                    break;
                } else if (strstr(buf, "OK") != NULL) {
                }
            } else if (bytes == 0) {
                printf("Дочерний процесс завершился.\n");
                break;
            }
        }

        close(parent_to_child[1]);
        close(child_to_parent[0]);

        wait(NULL);
        printf("Родительский процесс завершен.\n");
    } break;
    }

    return 0;
}