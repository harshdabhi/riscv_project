/**
 * @file riscv_sha256_eval_final.c
 * @brief A C program for SHA-256 that can be compiled WITH or WITHOUT
 * RISC-V crypto extensions for performance comparison.
 *
 * This version runs an automated benchmark sweep, hashing files of
 * increasing size and saving the results directly to a CSV file.
 *
 * The accelerated path uses the Zksh extension for the core SHA-256
 * logical functions (sum0, sum1, sig0, sig1).
 *
 * --- How to Compile ---
 *
 * 1. Compile WITHOUT acceleration (baseline):
 * The compiler will only use standard C operations.
 *
 * riscv64-unknown-elf-gcc -O2 -march=rv64g -o sha256_standard_final riscv_sha256_eval_final.c
 *
 * 2. Compile WITH RISC-V acceleration:
 * We enable the extensions and define the USE_RISCV_CRYPTO_EXT macro.
 *
 * riscv64-unknown-elf-gcc -O2 -march=rv64gc_zba_zbb_zbkb_zbkc_zbkx_zksh -D USE_RISCV_CRYPTO_EXT -o sha256_accelerated_final riscv_sha256_eval_final.c
 *
 * --- How to Run and Measure ---
 *
 * 1. Run the benchmark. It will create a CSV file automatically.
 *    qemu-riscv64 ./sha256_accelerated_final
 *    (This will create 'sha256_accelerated_results.csv')
 *
 * 2. To measure resource/energy usage, run under a monitoring tool like 'perf'.
 *    perf stat -e cycles,instructions ./sha256_accelerated_final
 *
 *    Correlate the 'perf' output with the generated CSV for a full analysis.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// SHA-256 constants
#define SHA256_BLOCK_SIZE 64
#define SHA256_DIGEST_SIZE 32

typedef struct {
    uint8_t  buf[SHA256_BLOCK_SIZE];
    uint32_t h[8];
    uint64_t len;
} sha256_ctx;

// Initial hash values for SHA-256
static const uint32_t sha256_h_init[8] = {
    0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
    0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
};

// Round constants for SHA-256
static const uint32_t sha256_k[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

// Helper for byte swapping
static inline uint32_t bswap_32(uint32_t x) {
    return ((x & 0xff000000) >> 24) | ((x & 0x00ff0000) >> 8) |
           ((x & 0x0000ff00) << 8)  | ((x & 0x000000ff) << 24);
}
static inline uint64_t bswap_64(uint64_t x) {
    return ((uint64_t)bswap_32(x) << 32) | bswap_32(x >> 32);
}

// SHA-256 transform function, processes one 64-byte block
void sha256_transform(sha256_ctx *ctx, const uint8_t *block);

// --- Public API ---
void sha256_init(sha256_ctx *ctx);
void sha256_update(sha256_ctx *ctx, const uint8_t *data, size_t len);
void sha256_final(sha256_ctx *ctx, uint8_t *digest);


/*****************************************************************************/
/* CORE SHA-256 TRANSFORM (STANDARD VS ACCELERATED)                          */
/*****************************************************************************/

#ifdef USE_RISCV_CRYPTO_EXT

// ACCELERATED VERSION (using Zksh instructions)

void sha256_transform(sha256_ctx *ctx, const uint8_t *block) {
    uint32_t w[16];
    uint32_t a, b, c, d, e, f, g, h;
    uint32_t t1, t2;

    memcpy(w, block, SHA256_BLOCK_SIZE);

    a = ctx->h[0]; b = ctx->h[1]; c = ctx->h[2]; d = ctx->h[3];
    e = ctx->h[4]; f = ctx->h[5]; g = ctx->h[6]; h = ctx->h[7];

    for (int i = 0; i < 64; ++i) {
        if (i >= 16) {
            uint32_t s0, s1;
            // s0 = sigma0(w[(i-15)&15])
            asm("sha256sig0 %0, %1" : "=r"(s0) : "r"(w[(i - 15) & 15]));
            // s1 = sigma1(w[(i-2)&15])
            asm("sha256sig1 %0, %1" : "=r"(s1) : "r"(w[(i - 2) & 15]));
            w[i & 15] = w[(i-16)&15] + s0 + w[(i-7)&15] + s1;
        }

        uint32_t s1, ch;
        // s1 = Sigma1(e)
        asm("sha256sum1 %0, %1" : "=r"(s1) : "r"(e));
        ch = (e & f) ^ (~e & g); // Choice function
        t1 = h + s1 + ch + sha256_k[i] + bswap_32(w[i & 15]);

        uint32_t s0, maj;
        // s0 = Sigma0(a)
        asm("sha256sum0 %0, %1" : "=r"(s0) : "r"(a));
        maj = (a & b) ^ (a & c) ^ (b & c); // Majority function
        t2 = s0 + maj;

        h = g; g = f; f = e; e = d + t1;
        d = c; c = b; b = a; a = t1 + t2;
    }

    ctx->h[0] += a; ctx->h[1] += b; ctx->h[2] += c; ctx->h[3] += d;
    ctx->h[4] += e; ctx->h[5] += f; ctx->h[6] += g; ctx->h[7] += h;
}

#else

// STANDARD C VERSION (baseline)

#define ROTR(x, n) (((x) >> (n)) | ((x) << (32 - (n))))
#define Ch(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
#define Maj(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define Sigma0(x) (ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22))
#define Sigma1(x) (ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25))
#define sigma0(x) (ROTR(x, 7) ^ ROTR(x, 18) ^ ((x) >> 3))
#define sigma1(x) (ROTR(x, 17) ^ ROTR(x, 19) ^ ((x) >> 10))

void sha256_transform(sha256_ctx *ctx, const uint8_t *block) {
    uint32_t w[64];
    uint32_t a, b, c, d, e, f, g, h;
    uint32_t t1, t2;

    for (int i = 0; i < 16; ++i) {
        w[i] = bswap_32(((uint32_t*)block)[i]);
    }
    for (int i = 16; i < 64; ++i) {
        w[i] = sigma1(w[i - 2]) + w[i - 7] + sigma0(w[i - 15]) + w[i - 16];
    }

    a = ctx->h[0]; b = ctx->h[1]; c = ctx->h[2]; d = ctx->h[3];
    e = ctx->h[4]; f = ctx->h[5]; g = ctx->h[6]; h = ctx->h[7];

    for (int i = 0; i < 64; ++i) {
        t1 = h + Sigma1(e) + Ch(e, f, g) + sha256_k[i] + w[i];
        t2 = Sigma0(a) + Maj(a, b, c);
        h = g; g = f; f = e; e = d + t1;
        d = c; c = b; b = a; a = t1 + t2;
    }

    ctx->h[0] += a; ctx->h[1] += b; ctx->h[2] += c; ctx->h[3] += d;
    ctx->h[4] += e; ctx->h[5] += f; ctx->h[6] += g; ctx->h[7] += h;
}

#endif // USE_RISCV_CRYPTO_EXT

/*****************************************************************************/
/* SHA-256 API IMPLEMENTATION (INIT, UPDATE, FINAL)                          */
/*****************************************************************************/

void sha256_init(sha256_ctx *ctx) {
    memcpy(ctx->h, sha256_h_init, sizeof(ctx->h));
    memset(ctx->buf, 0, sizeof(ctx->buf));
    ctx->len = 0;
}

void sha256_update(sha256_ctx *ctx, const uint8_t *data, size_t len) {
    size_t buffer_bytes = ctx->len % SHA256_BLOCK_SIZE;
    ctx->len += len;

    if (buffer_bytes > 0) {
        size_t to_fill = SHA256_BLOCK_SIZE - buffer_bytes;
        if (len < to_fill) {
            memcpy(ctx->buf + buffer_bytes, data, len);
            return;
        }
        memcpy(ctx->buf + buffer_bytes, data, to_fill);
        sha256_transform(ctx, ctx->buf);
        data += to_fill;
        len -= to_fill;
    }

    while (len >= SHA256_BLOCK_SIZE) {
        sha256_transform(ctx, data);
        data += SHA256_BLOCK_SIZE;
        len -= SHA256_BLOCK_SIZE;
    }

    if (len > 0) {
        memcpy(ctx->buf, data, len);
    }
}

void sha256_final(sha256_ctx *ctx, uint8_t *digest) {
    size_t buffer_bytes = ctx->len % SHA256_BLOCK_SIZE;

    ctx->buf[buffer_bytes++] = 0x80;

    if (buffer_bytes > SHA256_BLOCK_SIZE - 8) {
        memset(ctx->buf + buffer_bytes, 0, SHA256_BLOCK_SIZE - buffer_bytes);
        sha256_transform(ctx, ctx->buf);
        memset(ctx->buf, 0, SHA256_BLOCK_SIZE);
    } else {
        memset(ctx->buf + buffer_bytes, 0, SHA256_BLOCK_SIZE - buffer_bytes);
    }

    uint64_t bit_len = bswap_64(ctx->len * 8);
    memcpy(ctx->buf + SHA256_BLOCK_SIZE - 8, &bit_len, 8);
    sha256_transform(ctx, ctx->buf);

    for (int i = 0; i < 8; ++i) {
        ((uint32_t*)digest)[i] = bswap_32(ctx->h[i]);
    }
}


/*****************************************************************************/
/* AUTOMATED BENCHMARKING SUITE                                              */
/*****************************************************************************/

const char* TEMP_IN_FILENAME = "temp_data.bin";

typedef struct {
    double execution_time;
    double throughput_mbs;
} PerformanceResult;

int create_test_file(long long size) {
    FILE *fp = fopen(TEMP_IN_FILENAME, "wb");
    if (!fp) return -1;
    for (long long i = 0; i < size; ++i) fputc(i % 256, fp);
    fclose(fp);
    return 0;
}

PerformanceResult run_benchmark_for_size(long long size) {
    PerformanceResult result = {0.0, 0.0};
    if (create_test_file(size) != 0) {
        perror("ERROR: Failed to create test file");
        return result;
    }

    FILE *in_file = fopen(TEMP_IN_FILENAME, "rb");
    if (!in_file) {
        perror("ERROR: Error opening file for hashing");
        return result;
    }

    clock_t start = clock();

    sha256_ctx ctx;
    uint8_t file_buffer[4096];
    uint8_t final_hash[SHA256_DIGEST_SIZE];
    size_t bytes_read;

    sha256_init(&ctx);
    while((bytes_read = fread(file_buffer, 1, sizeof(file_buffer), in_file)) > 0) {
        sha256_update(&ctx, file_buffer, bytes_read);
    }
    sha256_final(&ctx, final_hash);

    clock_t end = clock();

    fclose(in_file);
    remove(TEMP_IN_FILENAME);

    result.execution_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    if (result.execution_time > 0) {
        result.throughput_mbs = (double)size / (1024 * 1024) / result.execution_time;
    }
    return result;
}

int main() {
    const long long START_SIZE = 100 * 1024;
    const long long END_SIZE   = 10 * 1024 * 1024;
    const long long STEP_SIZE  = 100 * 1024;

    printf("--- RISC-V SHA-256 Performance Sweep ---\n");
#ifdef USE_RISCV_CRYPTO_EXT
    const char* mode_str = "Accelerated (Zksh)";
    const char* csv_filename = "sha256_accelerated_results.csv";
#else
    const char* mode_str = "Standard C (Baseline)";
    const char* csv_filename = "sha256_standard_results.csv";
#endif
    printf("Mode: %s\n", mode_str);
    printf("Workload: Hashing files from %lld KB to %lld MB.\n", START_SIZE / 1024, END_SIZE / (1024 * 1024));

    FILE* csv_file = fopen(csv_filename, "w");
    if (!csv_file) {
        perror("ERROR: Could not open CSV file for writing");
        return 1;
    }
    fprintf(csv_file, "FileSize_KB,ExecutionTime_s,Throughput_MBps,CPU_Cycles_Placeholder,Energy_Joules_Placeholder\n");

    for (long long current_size = START_SIZE; current_size <= END_SIZE; current_size += STEP_SIZE) {
        printf("Processing size: %lld KB\r", current_size / 1024);
        fflush(stdout);

        PerformanceResult result = run_benchmark_for_size(current_size);

        fprintf(csv_file, "%lld,%.6f,%.2f,0.0,0.0\n",
                current_size / 1024,
                result.execution_time,
                result.throughput_mbs);
    }

    fclose(csv_file);

    printf("\n--- Benchmark Sweep Complete ---\n");
    printf("Results have been saved to '%s'.\n", csv_filename);

    return 0;
}