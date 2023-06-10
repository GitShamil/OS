#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>

#define SERVER_PORT 8080
#define BUFFER_SIZE 1024

char books[][20] = {"Book1", "Book2", "Book3", "Book4", "Book5"};
int availableBooks[sizeof(books) / sizeof(books[0])];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *clientHandler(void *arg) {
    int clientSocket = *(int *)arg;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    // Получение запросов от клиента
    while (1) {
        // Проверка наличия доступных книг
        int found = 0;
        for (int i = 0; i < sizeof(availableBooks) / sizeof(availableBooks[0]); i++) {
            if (availableBooks[i] == 1) {
                found = 1;
                break;
            }
        }

        if (found == 0) {
            // Отправка сообщения о том, что книги нет в наличии
            sendto(clientSocket, "NoBook", strlen("NoBook"), 0, (struct sockaddr *)&clientAddr, clientAddrLen);
            sleep(1); // Имитация ожидания книги
            continue;
        }

        // Нахождение доступной книги и отправка ее клиенту
        pthread_mutex_lock(&mutex);
        for (int i = 0; i < sizeof(availableBooks) / sizeof(availableBooks[0]); i++) {
            if (availableBooks[i] == 1) {
                strcpy(buffer, books[i]);
                printf("Библиотека: отдаю пользователю книгу %s!\n", buffer);
                availableBooks[i] = 0; // Книга недоступна
                break;
            }
        }
        pthread_mutex_unlock(&mutex);

        sendto(clientSocket, buffer, strlen(buffer), 0, (struct sockaddr *)&clientAddr, clientAddrLen);

        // Ожидание возврата книги от клиента
        memset(buffer, 0, BUFFER_SIZE);
        int receivedBytes = recvfrom(clientSocket, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&clientAddr, &clientAddrLen);
        if (receivedBytes <= 0)
            break;

        // Пометка книги как доступной
        pthread_mutex_lock(&mutex);
        printf("Книга доступна!\n");
        for (int i = 0; i < sizeof(books) / sizeof(books[0]); i++) {
            if (strcmp(books[i], buffer) == 0) {
                availableBooks[i] = 1; // Книга доступна
                break;
            }
        }
        pthread_mutex_unlock(&mutex);
    }

    // Закрытие сокета
    close(clientSocket);
    free(arg);

    return NULL;
}

int main() {
    int serverSocket;
    struct sockaddr_in serverAddr;

    // Создание сокета
    if ((serverSocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Ошибка создания сокета");
        exit(EXIT_FAILURE);
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    // Привязка сокета к адресу и порту
    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Ошибка привязки сокета");
        exit(EXIT_FAILURE);
    }

    printf("Библиотекарь: Сервер запущен и ожидает запросы...\n");

    // Инициализация доступности книг
    for (int i = 0; i < sizeof(availableBooks) / sizeof(availableBooks[0]); i++) {
        availableBooks[i] = 1; // Книги доступны
    }

    while (1) {
        // Принятие запроса от клиента
        char buffer[BUFFER_SIZE];
        struct sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        int receivedBytes = recvfrom(serverSocket, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&clientAddr, &clientAddrLen);
        if (receivedBytes <= 0)
            continue;

        printf("Библиотекарь: Клиент подключен. Создание отдельного потока для обработки запросов.\n");

        // Создание отдельного потока для обработки запросов клиента
        int *newSocket = (int *)malloc(sizeof(int));
        *newSocket = serverSocket;
        pthread_t tid;
        if (pthread_create(&tid, NULL, clientHandler, (void *)newSocket) < 0) {
            perror("Ошибка создания потока");
            exit(EXIT_FAILURE);
        }

        // Отсоединение потока
        pthread_detach(tid);
    }

    // Закрытие сокета
    close(serverSocket);

    return 0;
}
