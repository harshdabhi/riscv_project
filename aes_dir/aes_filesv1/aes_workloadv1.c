#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <sys/stat.h>

#define AES_BLOCK_SIZE 16

// Stub AES encrypt function — replace with your real AES implementation
void aes_encrypt(const uint8_t *in, uint8_t *out, const uint8_t *key) {
    // Dummy copy for placeholder
    for (int i = 0; i < AES_BLOCK_SIZE; i++)
        out[i] = in[i] ^ key[i];
}

void create_directories(const char *path) {
    char tmp[256];
    snprintf(tmp, sizeof(tmp), "%s", path);
    size_t len = strlen(tmp);
    if (tmp[len - 1] == '/') tmp[len - 1] = 0;

    for (char *p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = 0;
            mkdir(tmp, 0700);
            *p = '/';
        }
    }
    mkdir(tmp, 0700);
}

void generate_random_file(const char *filename, size_t size) {
    FILE *f = fopen(filename, "wb");
    if (!f) exit(1);
    uint8_t *buf = malloc(size);
    for (size_t i = 0; i < size; i++) buf[i] = rand() % 256;
    fwrite(buf, 1, size, f);
    free(buf);
    fclose(f);
}

void log_performance(double cpu_time_used, double throughput, size_t file_size) {
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);

    long maxrss = usage.ru_maxrss;
    long total_mem = sysconf(_SC_PHYS_PAGES) * sysconf(_SC_PAGE_SIZE) / 1024 / 1024;
    double ram_util = (double)maxrss / (total_mem * 1024) * 100;

    FILE *out = fopen("./generated_stat_files/performance_output_aes.csv", "a");
    if (!out) return;

    if (ftell(out) == 0) {
        fprintf(out, "File Size (KB), RAM Utilization, Max RSS (KB), Total RAM (MB), CPU Time Used (s), Throughput (B/s)\n");
    }

    fprintf(out, "%zu, %.2f, %ld, %ld, %.6f, %.2f\n",
            file_size / 1024, ram_util, maxrss, total_mem,
            cpu_time_used, throughput);

    fclose(out);
}

int main() {
    srand(time(NULL));
    create_directories("./bin/generated_files");
    create_directories("./generated_stat_files");

    uint8_t key[AES_BLOCK_SIZE] = {
        0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
        0xab, 0xf7, 0x98, 0x4d, 0x2b, 0x7e, 0x15, 0x16
    };

    for (size_t file_size = 10 * 1024; file_size <= 1024 * 1024; file_size += 10 * 1024) {
        char filename[256];
        snprintf(filename, sizeof(filename), "./bin/generated_files/random_%zu.dat", file_size);
        generate_random_file(filename, file_size);

        FILE *f = fopen(filename, "rb");
        if (!f) continue;

        uint8_t *data = malloc(file_size);
        fread(data, 1, file_size, f);
        fclose(f);

        uint8_t *encrypted = malloc(file_size);

        clock_t start = clock();
        for (size_t i = 0; i < file_size; i += AES_BLOCK_SIZE) {
            aes_encrypt(data + i, encrypted + i, key);
        }
        clock_t end = clock();

        double cpu_time = (double)(end - start) / CLOCKS_PER_SEC;
        double throughput = (double)file_size / cpu_time;

        log_performance(cpu_time, throughput, file_size);

        free(data);
        free(encrypted);
    }

    return 0;
}
