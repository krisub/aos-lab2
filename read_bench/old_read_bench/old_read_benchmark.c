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
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <filename> <num_reads>\n", argv[0]);
        return 1;
    }
    
    char *filename = argv[1];
    int num_reads = atoi(argv[2]);
    char buffer[BUFFER_SIZE];
    
    printf("Reading file: %s\n", filename);
    printf("Number of reads: %d\n", num_reads);

    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    printf("\n");
    printf("Read#,Time(ms),Cumulative(ms)\n");
    
    double start_total = get_time_ms();
    double cumulative = 0.0;
    
    for (int i = 0; i < num_reads; i++) {
        if (lseek(fd, 0, SEEK_SET) == -1) {
            perror("lseek");
            close(fd);
            return 1;
        }

        double start = get_time_ms();
        
        ssize_t bytes_read;
        off_t total_bytes = 0;
        
        while ((bytes_read = read(fd, buffer, BUFFER_SIZE)) > 0) {
            total_bytes += bytes_read;
        }
        
        if (bytes_read == -1) {
            perror("read");
            close(fd);
            return 1;
        }
        
        double end = get_time_ms();
        double elapsed = end - start;
        cumulative += elapsed;
        
        printf("%d,%.3f,%.3f\n", i + 1, elapsed, cumulative);
        fflush(stdout);
    }

    close(fd);

    double total_time = get_time_ms() - start_total;
    
    printf("\n");
    printf("Total time: %.3f ms\n", total_time);
    printf("Average per read: %.3f ms\n", total_time / num_reads);
    
    return 0;
}