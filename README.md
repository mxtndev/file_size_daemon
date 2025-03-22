# file_size_daemon

Конфигурационный файл (daemon.conf):
```
file /path/to/your/file.txt
socket /tmp/daemon_socket
```
Компиляция:
```
gcc -Wall -Wextra -Wpedantic -std=c11 -o file_size_daemon file_size_daemon.c
```
Запуск:
В интерактивном режиме:
```
./file_size_daemon
```
В фоновом режиме (демонизация):
```
./file_size_daemon -d
```
