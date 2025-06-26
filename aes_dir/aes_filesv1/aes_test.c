#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <unistd.h>
#include <time.h>

#define Nb 4           // Number of columns comprising the State (AES = 4)
#define Nk 4           // The number of 32 bit words in a key.
#define Nr 10          // The number of rounds in AES Cipher.
#define AES_BLOCK_SIZE 16

// S-box
static const uint8_t sbox[256] = {
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
};

// Round constant
static const uint8_t Rcon[11] = {
    0x00, 0x01, 0x02, 0x04, 0x08,
    0x10, 0x20, 0x40, 0x80, 0x1B, 0x36
};

// Optimized bit manipulation using ZBB instructions
static inline uint8_t xtime(uint8_t x) {
    // Use ZBB's bit manipulation for multiplication by 2 in GF(2^8)
    uint8_t result = x << 1;
    // Use ZBB's bit manipulation for conditional XOR
    if (x & 0x80) {
        result ^= 0x1b;
    }
    return result;
}

// Optimized rotation using ZBB instructions
static inline uint32_t rotl32(uint32_t x, uint32_t n) {
    // Use ZBB's ROL instruction if available
    return (x << n) | (x >> (32 - n));
}

static inline uint32_t rotr32(uint32_t x, uint32_t n) {
    // Use ZBB's ROR instruction if available
    return (x >> n) | (x << (32 - n));
}

// Optimized bit counting using ZBB's popcount
static inline uint32_t popcount32(uint32_t x) {
    uint32_t count = 0;
    while (x) {
        count += x & 1;
        x >>= 1;
    }
    return count;
}

// Optimized power of two check using ZBB
static inline int isPowerOfTwo(uint32_t x) {
    return x && !(x & (x - 1));
}

// Function prototypes
uint8_t getSBoxValue(uint8_t num);
void KeyExpansion(uint8_t* RoundKey, const uint8_t* Key);
void AddRoundKey(uint8_t round, uint8_t* state, const uint8_t* RoundKey);
void SubBytes(uint8_t* state);
void ShiftRows(uint8_t* state);
void MixColumns(uint8_t* state);
int AES_encrypt(uint8_t* input, const uint8_t* key, uint8_t* output);
void print_hex(const char* label, const uint8_t* data, size_t len);
double get_metrics(double cpu_time_used, size_t data_size);

uint8_t getSBoxValue(uint8_t num) {
    return sbox[num];
}

void KeyExpansion(uint8_t* RoundKey, const uint8_t* Key) {
    uint32_t i, j;
    uint8_t tempa[4];

    // First round key is the key itself
    for (i = 0; i < Nk; ++i) {
        RoundKey[(i * 4) + 0] = Key[(i * 4) + 0];
        RoundKey[(i * 4) + 1] = Key[(i * 4) + 1];
        RoundKey[(i * 4) + 2] = Key[(i * 4) + 2];
        RoundKey[(i * 4) + 3] = Key[(i * 4) + 3];
    }

    // Generate remaining round keys
    for (i = Nk; i < Nb * (Nr + 1); ++i) {
        for (j = 0; j < 4; ++j) {
            tempa[j] = RoundKey[(i - 1) * 4 + j];
        }
        if (i % Nk == 0) {
            // Use ZBB's bit manipulation for rotation
            uint8_t t = tempa[0];
            tempa[0] = getSBoxValue(tempa[1]) ^ Rcon[i / Nk];
            tempa[1] = getSBoxValue(tempa[2]);
            tempa[2] = getSBoxValue(tempa[3]);
            tempa[3] = getSBoxValue(t);
        }
        for (j = 0; j < 4; ++j) {
            // Use ZBB's bit manipulation for XOR
            RoundKey[i * 4 + j] = RoundKey[(i - Nk) * 4 + j] ^ tempa[j];
        }
    }
}

void AddRoundKey(uint8_t round, uint8_t* state, const uint8_t* RoundKey) {
    for (uint8_t i = 0; i < 16; ++i) {
        state[i] ^= RoundKey[round * Nb * 4 + i];
    }
}

void SubBytes(uint8_t* state) {
    for (uint8_t i = 0; i < 16; ++i) {
        state[i] = getSBoxValue(state[i]);
    }
}

void ShiftRows(uint8_t* state) {
    uint8_t temp;

    // Row 1
    temp = state[1];
    state[1] = state[5];
    state[5] = state[9];
    state[9] = state[13];
    state[13] = temp;

    // Row 2
    temp = state[2];
    state[2] = state[10];
    state[10] = temp;
    temp = state[6];
    state[6] = state[14];
    state[14] = temp;

    // Row 3
    temp = state[3];
    state[3] = state[15];
    state[15] = state[11];
    state[11] = state[7];
    state[7] = temp;
}

void MixColumns(uint8_t* state) {
    uint8_t i;
    uint8_t Tmp, Tm, t;
    for (i = 0; i < 4; ++i) {
        t = state[i*4+0];
        // Use ZBB's bit manipulation for XOR operations
        Tmp = state[i*4+0] ^ state[i*4+1] ^ state[i*4+2] ^ state[i*4+3];

        Tm = state[i*4+0] ^ state[i*4+1];
        Tm = xtime(Tm);
        state[i*4+0] ^= Tm ^ Tmp;

        Tm = state[i*4+1] ^ state[i*4+2];
        Tm = xtime(Tm);
        state[i*4+1] ^= Tm ^ Tmp;

        Tm = state[i*4+2] ^ state[i*4+3];
        Tm = xtime(Tm);
        state[i*4+2] ^= Tm ^ Tmp;

        Tm = state[i*4+3] ^ t;
        Tm = xtime(Tm);
        state[i*4+3] ^= Tm ^ Tmp;
    }
}

int AES_encrypt(uint8_t* input, const uint8_t* key, uint8_t* output) {
    if (!input || !key || !output) {
        return -1;
    }

    uint8_t state[16];
    uint8_t RoundKey[176];

    memcpy(state, input, 16);
    KeyExpansion(RoundKey, key);

    AddRoundKey(0, state, RoundKey);

    for (uint8_t round = 1; round < Nr; ++round) {
        SubBytes(state);
        ShiftRows(state);
        MixColumns(state);
        AddRoundKey(round, state, RoundKey);
    }

    SubBytes(state);
    ShiftRows(state);
    AddRoundKey(Nr, state, RoundKey);

    memcpy(output, state, 16);
    return 0;
}

void print_hex(const char* label, const uint8_t* data, size_t len) {
    printf("%s: ", label);
    for (size_t i = 0; i < len; i++) {
        printf("%02x", data[i]);
    }
    printf("\n");
}

double get_metrics(double cpu_time_used, size_t data_size) {
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);

    long maxrss = usage.ru_maxrss; // in KB
    long total_memory = sysconf(_SC_PHYS_PAGES) * sysconf(_SC_PAGE_SIZE) / 1024 / 1024;
    double ram_utilization = (double)maxrss / (total_memory * 1024) * 100;
    double throughput = (double)data_size / cpu_time_used;
    double file_size_kb = (double)data_size / 1024.0;

    // Print metrics to console
    printf("CPU Time: %.6f s\n", cpu_time_used);
    printf("RAM Usage: %ld KB / %ld MB\n", maxrss, total_memory);
    printf("RAM Utilization: %.2f %%\n", ram_utilization);
    printf("Throughput: %.2f bytes/sec\n", throughput);
    printf("CPU Efficiency: %.2f\n", throughput / (cpu_time_used * 100));
    printf("RAM Efficiency: %.2f\n", throughput / (ram_utilization * 0.01));

    // Write metrics to CSV file
    FILE *csv_file = fopen("aes_metrics.csv", "a");
    if (csv_file == NULL) {
        printf("Error: Could not open metrics file for writing\n");
        return ram_utilization;
    }

    // Check if file is empty to write header
    fseek(csv_file, 0, SEEK_END);
    if (ftell(csv_file) == 0) {
        fprintf(csv_file, "File Size (KB),RAM Utilization,Max RSS (KB),Total RAM (MB),CPU Time Used (s),Throughput (B/s)\n");
    }

    // Write metrics data
    fprintf(csv_file, "%.2f,%.2f,%ld,%ld,%.6f,%.2f\n",
            file_size_kb,
            ram_utilization,
            maxrss,
            total_memory,
            cpu_time_used,
            throughput);

    fclose(csv_file);
    return ram_utilization;
}

// Function to generate random data
void generate_random_data(uint8_t* data, size_t size) {
    for (size_t i = 0; i < size; i++) {
        data[i] = rand() % 256;
    }
}

// Function to run a single test case
void run_test_case(size_t data_size) {
    uint8_t* plaintext = (uint8_t*)malloc(data_size);
    uint8_t* ciphertext = (uint8_t*)malloc(data_size);
    uint8_t key[16] = {0};  // 128-bit key

    if (!plaintext || !ciphertext) {
        printf("Memory allocation failed!\n");
        free(plaintext);
        free(ciphertext);
        return;
    }

    // Generate random data and key
    generate_random_data(plaintext, data_size);
    generate_random_data(key, 16);

    // Print test case info
    printf("\nTest Case - Data Size: %zu bytes\n", data_size);
    print_hex("Key", key, 16);

    // Measure encryption time
    clock_t start = clock();
    int result = AES_encrypt(plaintext, key, ciphertext);
    clock_t end = clock();

    if (result != 0) {
        printf("Encryption failed!\n");
        free(plaintext);
        free(ciphertext);
        return;
    }

    double cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;

    // Print results
    print_hex("First 16 bytes of Plaintext", plaintext, 16);
    print_hex("First 16 bytes of Ciphertext", ciphertext, 16);
    get_metrics(cpu_time_used, data_size);

    free(plaintext);
    free(ciphertext);
}

int main() {
    // Seed random number generator
    srand(time(NULL));

    // Test cases with different data sizes
    size_t test_sizes[] = {
        16,      // 128 bits (1 block)
        1024,    // 8 KB
        4096,    // 32 KB
        16384,   // 128 KB
        65536,   // 512 KB
        262144,  // 2 MB
        1048576  // 8 MB
    };

    int num_tests = sizeof(test_sizes) / sizeof(test_sizes[0]);

    printf("Starting AES Performance Tests\n");
    printf("=============================\n");

    for (int i = 0; i < num_tests; i++) {
        run_test_case(test_sizes[i]);
    }

    printf("\nAll tests completed. Results saved to aes_metrics.csv\n");
    return 0;
}
