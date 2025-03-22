# Имя исполняемого файла
TARGET = file_size_daemon

# Компилятор
CC = gcc

# Флаги компиляции
CFLAGS = -Wall -Wextra -Wpedantic -std=c11

# Исходный файл
SRC = file_size_daemon.c

# Объектный файл (не используется напрямую, но можно добавить для многофайловых проектов)
OBJ = file_size_daemon.o

# Правило для сборки программы
all: $(TARGET)

$(TARGET): $(SRC)
    $(CC) $(CFLAGS) -o $(TARGET) $(SRC)

# Правило для запуска демона в фоновом режиме
run-daemon: $(TARGET)
    ./$(TARGET) -d

# Правило для запуска демона в интерактивном режиме
run: $(TARGET)
    ./$(TARGET)

# Правило для очистки временных файлов
clean:
    rm -f $(TARGET) *.o

# Правило для полной очистки (включая сокет, если он существует)
distclean: clean
    rm -f /tmp/daemon_socket

.PHONY: all run run-daemon clean distclean