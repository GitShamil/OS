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
    int fd2, fd1, result;
    // размер который получилось записать в pipe
    int size;

    mknod(name_of_pipe1, S_IFIFO | 0666, 0);
    mknod(name_of_pipe2, S_IFIFO | 0666, 0);

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
            return 0;
}
