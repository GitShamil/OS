#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main() {
    int fd1, fd2;
    char buf[1] = {'2'};

    if (mkfifo("channel2", 0666) == -1) {
        return 1;
    }

    fd1 = open("channel2", O_WRONLY);
    if (fd1 == -1) {
        return 1;
    }

    if (write(fd1, buf, 1) == -1) {
        return 1;
    }

    fd2 = open("channel2", O_RDONLY);
    if (fd2 == -1) {
        return 1;
    }

    if (read(fd2, buf, 1) == -1) {
        return 1;
    }

    printf("%c", buf[0]);

    close(fd1);
    close(fd2);

    return 0;
}