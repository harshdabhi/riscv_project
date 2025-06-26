#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#define SHA256_BLOCK_SIZE 32

const uint32_t K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0x80a1a1d6, 0x6ca6351e, 0x1f83d9ab, 0x5be0cd19, 0x71c58e8b, 0x4958d65e, 0x4065c8a9,
    0x528c45cc, 0x1c4c6c9d, 0xa34d23d6, 0xe58bcd3d, 0x0a372f1b, 0x24ef28b2, 0x648832c6, 0x0de82b8d,
    0x0abf9e51, 0x45763bcd, 0x32b41b0e, 0x17f0b930, 0x2f017f1a, 0x3bcf5c6f, 0x655e17ad, 0x1893c3e3,
    0x73d734b9, 0xd0636817, 0x4f032b24, 0xbafc66dd, 0xacaf6481, 0x45ac767d, 0xe9a7b617, 0x60e94a35,
    0x75945616, 0xa137e0d5, 0x9dbf1c68, 0x41805497, 0x4e875e4c, 0x944d2f91, 0x70bb2b6c, 0x0768c2a3
};

#define ROTRIGHT(a, b) (((a) >> (b)) | ((a) << (32 - (b))))
#define CH(x, y, z) ((x & y) ^ (~x & z))
#define MAJ(x, y, z) ((x & y) ^ (x & z) ^ (y & z))
#define SIGMA0(x) (ROTRIGHT(x, 2) ^ ROTRIGHT(x, 13) ^ ROTRIGHT(x, 22))
#define SIGMA1(x) (ROTRIGHT(x, 6) ^ ROTRIGHT(x, 11) ^ ROTRIGHT(x, 25))
#define sigma0(x) (ROTRIGHT(x, 7) ^ ROTRIGHT(x, 18) ^ ((x) >> 3))
#define sigma1(x) (ROTRIGHT(x, 17) ^ ROTRIGHT(x, 19) ^ ((x) >> 10))

void sha256_transform(uint32_t state[8], const uint8_t data[64]) {
    uint32_t a, b, c, d, e, f, g, h, t1, t2, m[64];

    for (int i = 0; i < 16; ++i) {
        m[i] = (data[i * 4] << 24) | (data[i * 4 + 1] << 16) |
               (data[i * 4 + 2] << 8) | data[i * 4 + 3];
    }
    for (int i = 16; i < 64; ++i) {
        m[i] = sigma1(m[i - 2]) + m[i - 7] + sigma0(m[i - 15]) + m[i - 16];
    }

    a = state[0]; b = state[1]; c = state[2]; d = state[3];
    e = state[4]; f = state[5]; g = state[6]; h = state[7];

    for (int i = 0; i < 64; ++i) {
        t1 = h + SIGMA1(e) + CH(e, f, g) + K[i] + m[i];
        t2 = SIGMA0(a) + MAJ(a, b, c);
        h = g; g = f; f = e; e = d + t1;
        d = c; c = b; b = a; a = t1 + t2;
    }

    state[0] += a; state[1] += b; state[2] += c; state[3] += d;
    state[4] += e; state[5] += f; state[6] += g; state[7] += h;
}

void sha256_init(uint32_t state[8]) {
    state[0] = 0x6a09e667; state[1] = 0xbb67ae85; state[2] = 0x3c6ef372;
    state[3] = 0xa54ff53a; state[4] = 0x510e527f; state[5] = 0x9b05688c;
    state[6] = 0x1f83d9ab; state[7] = 0x5be0cd19;
}

void sha256_update(uint32_t state[8], const uint8_t *data, size_t len) {
    while (len >= 64) {
        sha256_transform(state, data);
        data += 64;
        len -= 64;
    }
}

void sha256_final(uint32_t state[8], const uint8_t *data, size_t len, uint8_t *hash) {
    uint8_t block[64] = {0};
    size_t rem = len % 64;
    memcpy(block, data + (len - rem), rem);
    block[rem] = 0x80;

    if (rem > 55) {
        sha256_update(state, block, 64);
        memset(block, 0, 64);
    }

    uint64_t bit_len = len * 8;
    for (int i = 0; i < 8; i++) {
        block[63 - i] = (bit_len >> (8 * i)) & 0xff;
    }

    sha256_update(state, block, 64);

    for (int i = 0; i < 8; i++) {
        hash[i * 4 + 0] = (state[i] >> 24) & 0xFF;
        hash[i * 4 + 1] = (state[i] >> 16) & 0xFF;
        hash[i * 4 + 2] = (state[i] >> 8) & 0xFF;
        hash[i * 4 + 3] = (state[i]) & 0xFF;
    }
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
    FILE *file = fopen(filename, "wb");
    if (!file) {
        perror("File open failed");
        exit(1);
    }

    unsigned char *buffer = malloc(size);
    if (!buffer) {
        perror("Memory allocation failed");
        fclose(file);
        exit(1);
    }

    for (size_t i = 0; i < size; ++i) buffer[i] = rand() % 256;
    fwrite(buffer, 1, size, file);
    free(buffer);
    fclose(file);
}

void log_performance(double cpu_time_used, double throughput, uint8_t *hash, size_t file_size) {
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);

    long maxrss = usage.ru_maxrss;
    long total_memory = sysconf(_SC_PHYS_PAGES) * sysconf(_SC_PAGE_SIZE) / 1024 / 1024;
    double ram_utilization = (double)maxrss / (total_memory * 1024) * 100;

    FILE *output_file = fopen("./generated_stat_files/performance_output_sha.csv", "a");
    if (!output_file) return;

    if (ftell(output_file) == 0) {
        fprintf(output_file, "File Size (KB), RAM Utilization, Max RSS (KB), Total RAM (MB), User Time, System Time, CPU Time Used (s), Throughput (B/s), SHA-256 Hash\n");
    }

    fprintf(output_file, "%zu, %.2f, %ld, %ld, %.6f, %.6f, %.6f, %.6f, ",
        file_size / 1024, ram_utilization, maxrss, total_memory,
        usage.ru_utime.tv_sec + usage.ru_utime.tv_usec / 1e6,
        usage.ru_stime.tv_sec + usage.ru_stime.tv_usec / 1e6,
        cpu_time_used, throughput);

    for (int i = 0; i < SHA256_BLOCK_SIZE; i++)
        fprintf(output_file, "%02x", hash[i]);
    fprintf(output_file, "\n");

    fclose(output_file);
}

int main() {
    srand(time(NULL));

    create_directories("./bin/generated_files");
    create_directories("./generated_stat_files");

    for (size_t file_size = 10 * 1024; file_size <= 1024 * 1024; file_size += 10 * 1024) {
        char filename[256];
        snprintf(filename, sizeof(filename), "./bin/generated_files/random_file_%zu.dat", file_size);

        generate_random_file(filename, file_size);

        FILE *fp = fopen(filename, "rb");
        if (!fp) continue;

        uint8_t *data = malloc(file_size);
        if (!data) {
            fclose(fp);
            continue;
        }

        fread(data, 1, file_size, fp);
        fclose(fp);

        uint32_t state[8];
        uint8_t hash[SHA256_BLOCK_SIZE];
        clock_t start = clock();

        sha256_init(state);
        sha256_update(state, data, file_size);
        sha256_final(state, data, file_size, hash);

        clock_t end = clock();
        free(data);

        double cpu_time_used = (double)(end - start) / CLOCKS_PER_SEC;
        double throughput = (double)file_size / cpu_time_used;

        log_performance(cpu_time_used, throughput, hash, file_size);
    }

    return 0;
}
