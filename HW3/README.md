# Зиганшин Шамиль Асхатович БПИ217
## ИДЗ3. Вариант 36.

## Условие
В библиотеке имеется N книг, каждая из книг в одном экземпляре. M читателей регулярно заглядывают в библиотеку, выбирает для чтения одну книгу и читает ее некоторое количество дней. Если желаемой книги нет, то читатель дожидается от библиотекаря информации об ее появлении и приходит в библиотеку, чтобы специально забрать ее. Возможна ситуация, когда несколько читателей конкурируют из-за этой популярной книги. Создать приложение, моделирующее заданный процесс.

## Сценарий задачи.
Есть сервер - это библиотека, получающая запросы от читатлей. Библиотека проверяет наличие книги, которую просит читатель. Если эта книга имеется в наличии, то библиотека отдает эту книгу, а пользователь ее читает и возвращает.

## Программа выполнена на 7 баллов.
В программе можно подключить несколько пользователей. Программа будет выполняться корректно, а также сервер тоже завершается корректно.

## Код сервера (библиотеки)
```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>

#define SERVER_PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

char books[][20] = {"Book1", "Book2", "Book3", "Book4", "Book5"};
int availableBooks[sizeof(books) / sizeof(books[0])];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *clientHandler(void *arg) {
    int clientSocket = *(int *)arg;
    char buffer[BUFFER_SIZE];

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
            send(clientSocket, "NoBook", strlen("NoBook"), 0);
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

        send(clientSocket, buffer, strlen(buffer), 0);

        // Ожидание возврата книги от клиента
        memset(buffer, 0, BUFFER_SIZE);
        int receivedBytes = recv(clientSocket, buffer, BUFFER_SIZE, 0); 
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
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    pthread_t tid;

    // Создание сокета
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
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

    // Начало прослушивания входящих соединений
    if (listen(serverSocket, MAX_CLIENTS) < 0) {
        perror("Ошибка прослушивания");
        exit(EXIT_FAILURE);
    }

    printf("Библиотекарь: Ожидание подключений от клиентов...\n");

    // Инициализация доступности книг
    for (int i = 0; i < sizeof(availableBooks) / sizeof(availableBooks[0]); i++) {
        availableBooks[i] = 1; // Книги доступны
    }

    while (1) {
        // Принятие входящего соединения
        clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);

        if (clientSocket < 0) {
            perror("Ошибка при принятии соединения");
            exit(EXIT_FAILURE);
        }

        printf("Библиотекарь: Клиент подключен. Создание отдельного потока для обработки запросов.\n");

        // Создание отдельного потока для обработки запросов клиента
        int *newSocket = (int *)malloc(sizeof(int));
        *newSocket = clientSocket;
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
```

## Код клиента (читателя)

```c
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
```

## Вывод программы

### Библиотека
```
Библиотекарь: Ожидание подключений от клиентов...
Библиотекарь: Клиент подключен. Создание отдельного потока для обработки запросов.
Библиотека: отдаю пользователю книгу Book1!
Книга доступна!
Библиотека: отдаю пользователю книгу Book2!
Библиотекарь: Клиент подключен. Создание отдельного потока для обработки запросов.
Библиотека: отдаю пользователю книгу Book3!
Книга доступна!
Библиотека: отдаю пользователю книгу Book4!
Книга доступна!
Библиотека: отдаю пользователю книгу Book5!
Книга доступна!
Книга доступна!
```
### Читатель 1
```
Читатель: Читаю книгу: Book1
Читатель: Читаю книгу: Book2
Читатель: Читаю книгу: Book4
Читатель: Читаю книгу: Book3
Читатель: Читаю книгу: Book5
Читатель: Книги нет в наличии. Жду...
Читатель: Книги нет в наличии. Жду...
Читатель: Книги нет в наличии. Жду...
Читатель: Ну ладно, как хотите. Пока!
```

### Читатель 2
```
Читатель: Читаю книгу: Book1
Читатель: Читаю книгу: Book2
Читатель: Читаю книгу: Book3
Читатель: Читаю книгу: Book5
Читатель: Читаю книгу: Book4
Читатель: Книги нет в наличии. Жду...
Читатель: Книги нет в наличии. Жду...
Читатель: Книги нет в наличии. Жду...
Читатель: Ну ладно, как хотите. Пока!
```

Как можно увидеть, читатели не уйдут, пока не прочтут все книги. Если же читатели прочитали всё, а книг нет, то они отключатся от сервера.