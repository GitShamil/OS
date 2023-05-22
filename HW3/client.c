#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int socketId;
    struct sockaddr_in serverAddr;
    char buffer[BUFFER_SIZE];
    int cnt = 0;
    // Создание сокета
    if ((socketId = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Ошибка создания сокета");
        exit(EXIT_FAILURE);
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);

    // Преобразование IP-адреса из текстового в бинарный формат
    if (inet_pton(AF_INET, SERVER_IP, &(serverAddr.sin_addr)) <= 0) {
        perror("Ошибка преобразования адреса");
        exit(EXIT_FAILURE);
    }

    // Подключение к серверу
    if (connect(socketId, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Ошибка подключения");
        exit(EXIT_FAILURE);
    }

    // Цикл чтения книги
    while (1) {
        // Получение сообщения от сервера
        int receivedBytes = recv(socketId, buffer, BUFFER_SIZE, 0);
        if (receivedBytes <= 0)
            break;

        // Проверка наличия книги
        if (strcmp(buffer, "NoBook") == 0) {
            if (cnt == 3) {
                printf("Читатель: Ну ладно, как хотите. Пока!\n");
                break;
            }
            printf("Читатель: Книги нет в наличии. Жду...\n");
            cnt++;
            continue;
        }

        // Чтение книги
        printf("Читатель: Читаю книгу: %s\n", buffer);
        sleep(3); // Имитация чтения книги в течение нескольких секунд

        // Отправка сообщения о возвращении книги
        send(socketId, "BookReturned", strlen("BookReturned"), 0);
    }

    // Закрытие сокета
    close(socketId);

    return 0;
}
