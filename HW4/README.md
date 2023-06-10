# Зиганшин Шамиль Асхатович БПИ217
## ИДЗ4. Вариант 36.

## В чём отличие от прошлого задания?
Сейчас опишу все изменения с перехода TCP на UDP:

1. Изменился тип сокета при создании серверного сокета на `SOCK_DGRAM`, чтобы указать использование UDP: 
```c
if ((serverSocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("Ошибка создания сокета");
    exit(EXIT_FAILURE);
}
```

2. Удалены функции `listen` и `accept`, так как они не требуются для UDP. Вместо этого, сервер ожидает запросы от клиентов с помощью функции `recvfrom`, которая получает данные от клиента и информацию об адресе клиента:
```c
int receivedBytes = recvfrom(serverSocket, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&clientAddr, &clientAddrLen);
```

3. В функции `send` для отправки данных клиенту была заменена на `sendto`, чтобы указать адрес клиента, которому необходимо отправить данные:
```c
sendto(clientSocket, buffer, strlen(buffer), 0, (struct sockaddr *)&clientAddr, clientAddrLen);
```

4. Изменился тип сокета при создании клиентского сокета на `SOCK_DGRAM`:
```c
if ((socketId = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("Ошибка создания сокета");
    exit(EXIT_FAILURE);
}
```

5. Вместо функции `connect` для установления соединения с сервером, клиентский код просто отправляет запросы на сервер с помощью функции `sendto` и принимает ответы с помощью функции `recvfrom`, указывая адрес сервера в соответствующих параметрах.


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
    if ((socketId = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
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

    // Цикл чтения книги
    while (1) {
        // Получение сообщения от сервера
        socklen_t serverAddrLen = sizeof(serverAddr);
        int receivedBytes = recvfrom(socketId, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&serverAddr, &serverAddrLen);
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
        sendto(socketId, "BookReturned", strlen("BookReturned"), 0, (struct sockaddr *)&serverAddr, serverAddrLen);
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