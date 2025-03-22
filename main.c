#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <signal.h>

#define CONFIG_FILE "daemon.conf"
#define MAX_PATH_LEN 256
#define BUFFER_SIZE 1024

// Глобальные переменные для конфигурации
char file_path[MAX_PATH_LEN];
char socket_path[MAX_PATH_LEN];

// Функция для чтения конфигурации
int read_config() {
    FILE *config = fopen(CONFIG_FILE, "r");
    if (!config) {
        perror("Ошибка открытия конфигурационного файла");
        return -1;
    }

    char line[BUFFER_SIZE];
    while (fgets(line, sizeof(line), config)) {
        char key[BUFFER_SIZE], value[BUFFER_SIZE];
        if (sscanf(line, "%s %s", key, value) == 2) {
            if (strcmp(key, "file") == 0) {
                strncpy(file_path, value, MAX_PATH_LEN);
            } else if (strcmp(key, "socket") == 0) {
                strncpy(socket_path, value, MAX_PATH_LEN);
            }
        }
    }

    fclose(config);
    return 0;
}

// Функция для демонизации процесса
void daemonize() {
    pid_t pid = fork();
    if (pid < 0) {
        perror("Ошибка fork");
        exit(EXIT_FAILURE);
    }
    if (pid > 0) {
        exit(EXIT_SUCCESS); // Родительский процесс завершается
    }

    // Создаем новую сессию
    if (setsid() < 0) {
        perror("Ошибка setsid");
        exit(EXIT_FAILURE);
    }

    // Изменяем рабочую директорию на корневую
    if (chdir("/") < 0) {
        perror("Ошибка chdir");
        exit(EXIT_FAILURE);
    }

    // Закрываем стандартные файловые дескрипторы
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}

// Функция для получения размера файла
ssize_t get_file_size(const char *path) {
    struct stat st;
    if (stat(path, &st) == 0) {
        return st.st_size;
    }
    return -1;
}

// Основная функция демона
void run_daemon() {
    int server_fd, client_fd;
    struct sockaddr_un addr;

    // Удаляем старый сокет, если он существует
    unlink(socket_path);

    // Создаем UNIX-сокет
    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Ошибка создания сокета");
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Ошибка bind");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) < 0) {
        perror("Ошибка listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Демон запущен. Ожидание подключений...\n");

    while (1) {
        client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0) {
            perror("Ошибка accept");
            continue;
        }

        ssize_t size = get_file_size(file_path);
        char response[BUFFER_SIZE];

        if (size < 0) {
            snprintf(response, sizeof(response), "Ошибка: %s\n", strerror(errno));
        } else {
            snprintf(response, sizeof(response), "%zd\n", size);
        }

        write(client_fd, response, strlen(response));
        close(client_fd);
    }

    close(server_fd);
    unlink(socket_path);
}

int main(int argc, char *argv[]) {
    if (read_config() < 0) {
        fprintf(stderr, "Ошибка чтения конфигурации\n");
        return EXIT_FAILURE;
    }

    if (argc > 1 && strcmp(argv[1], "-d") == 0) {
        daemonize();
    }

    run_daemon();
    return EXIT_SUCCESS;
}