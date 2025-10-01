#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>

#define BUFFER_SIZE 4096
#define NUM_READS 10000
#define REPORT_INTERVAL 250 // interval to output progress into CSV

double get_time_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000.0) + (tv.tv_usec / 1000.0);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    char *filename = argv[1];
    char buffer[BUFFER_SIZE];
    struct stat st;

    // FUSE open does the one time scp
    double start_time = get_time_ms();

    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    if (fstat(fd, &st) == -1) {
        perror("fstat");
        close(fd);
        return 1;
    }

    off_t file_size = st.st_size;
    if (file_size < BUFFER_SIZE) {
        fprintf(stderr, "File is smaller than buffer size.\n");
        close(fd);
        return 1;
    }

    srand(time(NULL));

    // CSV header
    printf("ReadsCompleted,CumulativeTime(ms)\n");

    // 0: initial opening cost
    printf("0,%.3f\n", get_time_ms() - start_time);
    fflush(stdout);

    for (int i = 0; i < NUM_READS; i++) {
        off_t offset = rand() % (file_size - BUFFER_SIZE);

        if (pread(fd, buffer, BUFFER_SIZE, offset) != BUFFER_SIZE) {
            // skip
        }

        if ((i + 1) % REPORT_INTERVAL == 0) {
            double current_time = get_time_ms();
            printf("%d,%.3f\n", i + 1, current_time - start_time);
            fflush(stdout);
        }
    }

    close(fd);

    double total_time = get_time_ms() - start_time;

    fprintf(stderr, "\nBenchmark complete.\n");
    fprintf(stderr, "Total time for %d random reads: %.3f ms\n", NUM_READS, total_time);

    return 0;
}