#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>

#define BUFFER_SIZE 4096

double get_time_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000.0) + (tv.tv_usec / 1000.0);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file1> <file2> ... <fileN>\n", argv[0]);
        return 1;
    }

    char buffer[BUFFER_SIZE];
    double total_cumulative_time = 0.0;

    // CSV header
    printf("File,OperationTime(ms),CumulativeTime(ms)\n");

    for (int i = 1; i < argc; i++) {
        char *filename = argv[i];

        double start_time = get_time_ms();

        int fd = open(filename, O_RDONLY);
        if (fd == -1) {
            perror("open");
            fprintf(stderr, "Failed on file: %s\n", filename);
            continue; // skip to next file
        }

        // read only the first 4KB block
        ssize_t bytes_read = read(fd, buffer, BUFFER_SIZE);
        if (bytes_read == -1) {
            perror("read");
        }

        close(fd);

        double end_time = get_time_ms();
        double operation_time = end_time - start_time;
        total_cumulative_time += operation_time;

        printf("%s,%.3f,%.3f\n", filename, operation_time, total_cumulative_time);
        fflush(stdout);
    }
    
    fprintf(stderr, "\nBenchmark complete.\n");
    fprintf(stderr, "Total time for %d files: %.3f ms\n", argc - 1, total_cumulative_time);
    fprintf(stderr, "Average time per file: %.3f ms\n", total_cumulative_time / (argc - 1));

    return 0;
}