#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>


const int memory = 5000;

const char* name_of_pipe1 = "name_of_pipe1.fifo";
const char* name_of_pipe2 = "name_of_pipe2.fifo";

void is_palindrom(char line[]) {
    int size = 0;
    for (int i = 0; i < memory; ++i){
    	if (line[i] == '\0' || line[i] == '\n'){
    		size = i;
    		break;
    	}
    }
    char flag = '1';
    for (int i = 0; i < size / 2 + 1; ++i) {
        if (line[i] != line[size - 1 - i]){
            flag = '0';
        }
    }
    line[0] = flag;
}

int main(int argc, char *argv[]) {
    char buffer[memory];
    for(int i = 0; i < memory;i++){
    	buffer[i] = '\0';
    }
    if (argc != 3) {
        printf("Wrong format");
        return 0;
    }

    int fd2, fd1, result;
    // размер который получилось записать в pipe
    int size;

    // создаем канал для передачи из 1-го во 2-ой процесс
    mknod(name_of_pipe1, S_IFIFO | 0666, 0);
    // создаем канал для передачи из 2-го в 3-ий процесс
    mknod(name_of_pipe2, S_IFIFO | 0666, 0);

    // форкаем на 1-ый и 2-ой процесс
    result = fork();
    if (result < 0) {
        printf("something wrong with fork");
        exit(-1);
    } else if (result > 0) { // Первый процесс
        // файловый дескриптор на входной файл
        int file_to_read = 0;

        if ((file_to_read = open(argv[1], O_RDONLY, 0666)) < 0) {
            printf("Can not open file\n");
            exit(-1);
        }
        // считываем из файла
        size = read(file_to_read, buffer, memory);
        if (close(file_to_read) < 0) {
            printf("Can not close file\n");
        }
    	if ((fd1 = open(name_of_pipe1, O_WRONLY)) < 0){
            printf("Can't open pipe");
    	    exit(-1);
    	}
        // записываем в первый pipe
        size = write(fd1, buffer, memory);
        if (size != memory) {
            printf("Can not write all message to name_of_pipe1\n (size = %d)", size);
            exit(-1);
        }
        if (close(fd1) < 0) {
            printf("child: can not close writing to name_of_pipe1\n");
            exit(-1);
        }
        // второй процесс
    } else {
        int result1 = fork(); 
        if (result1 < 0) {
            printf("Can\'t fork parent\n");
            exit(-1);
        } else if (result1 > 0) { 
        // второй процесс
            // считываем из первого канала
            if ((fd1 = open(name_of_pipe1, O_RDONLY)) < 0){
            	printf("Can't open pipe");
            	exit(-1);
            }
            size = read(fd1, buffer, memory);
            if (size < 0) {
                printf("Can\'t read string from pipe\n");
                exit(-1);
            }
            if (close(fd1) < 0) {
                printf("parent: Can\'t close reading side of pipe\n");
                exit(-1);

            }
            // проверяем на палиндром
            is_palindrom(buffer);
            
            if ((fd2 = open(name_of_pipe2, O_WRONLY)) < 0){
           	 printf("Can't open pipe");
    	   	 exit(-1);
    	    }
            //memory = strlen(buffer);
            size = write(fd2, buffer, memory); // записываем во второй канал

            if (size != memory) {
                printf("Can\'t write all string to pipe\n (size = %d)", size);
                exit(-1);
            }
            if (close(fd2) < 0) {
                printf("parent: Can\'t close writing side of pipe\n");
                exit(-1);
            }
        } else { 
        	// 3-ий процесс
		// считываем из 2-го канала
     	    if ((fd2 = open(name_of_pipe2, O_RDONLY)) < 0){
            	printf("Can't open pipe");
            	exit(-1);
            }
            size = read(fd2, buffer, memory);
            if (size < 0) {
                printf("Can\'t read string from pipe\n");
                exit(-1);
            }
            if (close(fd2) < 0) {
                printf("child: Can\'t close reading side of pipe\n");
                exit(-1);
            }
            int file_to_write = 0;
         
            if ((file_to_write = open(argv[2], O_WRONLY | O_CREAT, 0666)) < 0) {
                printf("Can\'t open file\n");
                exit(-1);
            }
            size = write(file_to_write, buffer, 1); // записываем в файл
            if (size != 1) {
                printf("Can\'t write all string\n");
                exit(-1);
            }
            if (close(file_to_write) < 0) {
                printf("Can\'t close file\n");
            }
        }
    }
    return 0;
}
