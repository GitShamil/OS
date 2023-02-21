#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define BUFFER_SIZE 4096

int main(int argc, char *argv[])
{
    int fd_in, fd_out;
    char *input_file, *output_file;
    char buffer[BUFFER_SIZE];
    ssize_t nread;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input_file> <output_file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    input_file = argv[1];
    output_file = argv[2];

    /* Open the input file for reading */
    fd_in = open(input_file, O_RDONLY);
    if (fd_in == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    /* Open the output file for writing */
    fd_out = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd_out == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    /* Read the input file and write it to the output file */
    while ((nread = read(fd_in, buffer, BUFFER_SIZE)) > 0) {
        if (write(fd_out, buffer, nread) != nread) {
            perror("write");
            exit(EXIT_FAILURE);
        }
    }

    if (nread == -1) {
        perror("read");
        exit(EXIT_FAILURE);
    }

    close(fd_in);
    close(fd_out);

    return 0;
}
