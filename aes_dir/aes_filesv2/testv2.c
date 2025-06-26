/**
 * @file riscv_aes_eval_v2_file.c
 * @brief A C program for AES-128 file encryption that can be compiled
 * WITH or WITHOUT RISC-V crypto extensions for performance comparison.
 *
 * This version processes a file from disk, providing a more realistic
 * workload benchmark than an in-memory loop. It takes the file size
 * as a command-line argument.
 *
 * The accelerated path uses:
 * - 'rori' (Zbb) for the ShiftRows step.
 * - 'clmul' (Zbc) for the MixColumns step.
 *
 * --- How to Compile and Run ---
 *
 * 1. Compile WITHOUT acceleration (baseline):
 * riscv64-unknown-elf-gcc -O2 -march=rv64g -o aes_standard_file riscv_aes_eval_v2_file.c
 *
 * 2. Compile WITH RISC-V acceleration:
 * riscv64-unknown-elf-gcc -O2 -march=rv64gc_zba_zbb_zbc_zbs -D USE_RISCV_ACCEL -o aes_accelerated_file riscv_aes_eval_v2_file.c
 *
 * 3. Run and Compare Timings:
 * Execute both binaries with a desired file size (e.g., 10MB).
 *
 * # Test with 10 KB (10 * 1024)
 * qemu-riscv64 aes_standard_file 10240
 * qemu-riscv64 aes_accelerated_file 10240
 *
 * # Test with 1 MB (1 * 1024 * 1024)
 * qemu-riscv64 aes_standard_file 1048576
 * qemu-riscv64 aes_accelerated_file 1048576
 *
 * # Test with 10 MB (10 * 1024 * 1024)
 * qemu-riscv64 aes_standard_file 10485760
 * qemu-riscv64 aes_accelerated_file 10485760
 *
 * The 'aes_accelerated_file' binary should show significant throughput improvement.
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Defines for AES-128
#define Nk 4  // Number of 32-bit words in the key
#define Nb 4  // Number of columns (32-bit words) in the state
#define Nr 10 // Number of rounds for AES-128
#define AES_BLOCK_SIZE 16

// AES S-box
static const uint8_t s_box[256] = {
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
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16};

// AES Rcon
static const uint8_t Rcon[11] = {0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36};

// AES state
typedef uint8_t state_t[4][4];


/*****************************************************************************/
/* COMMON HELPERS & KEY EXPANSION                                            */
/*****************************************************************************/

void KeyExpansion(uint8_t* RoundKey, const uint8_t* Key) {
    uint32_t i, j, k;
    uint8_t tempa[4];
    for (i = 0; i < Nk; ++i) {
        RoundKey[(i * 4) + 0] = Key[(i * 4) + 0];
        RoundKey[(i * 4) + 1] = Key[(i * 4) + 1];
        RoundKey[(i * 4) + 2] = Key[(i * 4) + 2];
        RoundKey[(i * 4) + 3] = Key[(i * 4) + 3];
    }
    for (i = Nk; i < Nb * (Nr + 1); ++i) {
        k = (i - 1) * 4;
        tempa[0] = RoundKey[k + 0]; tempa[1] = RoundKey[k + 1];
        tempa[2] = RoundKey[k + 2]; tempa[3] = RoundKey[k + 3];
        if (i % Nk == 0) {
            const uint8_t u8tmp = tempa[0];
            tempa[0] = tempa[1]; tempa[1] = tempa[2];
            tempa[2] = tempa[3]; tempa[3] = u8tmp;
            tempa[0] = s_box[tempa[0]]; tempa[1] = s_box[tempa[1]];
            tempa[2] = s_box[tempa[2]]; tempa[3] = s_box[tempa[3]];
            tempa[0] = tempa[0] ^ Rcon[i / Nk];
        }
        j = i * 4; k = (i - Nk) * 4;
        RoundKey[j + 0] = RoundKey[k + 0] ^ tempa[0];
        RoundKey[j + 1] = RoundKey[k + 1] ^ tempa[1];
        RoundKey[j + 2] = RoundKey[k + 2] ^ tempa[2];
        RoundKey[j + 3] = RoundKey[k + 3] ^ tempa[3];
    }
}

void AddRoundKey(uint8_t round, state_t* state, const uint8_t* RoundKey) {
    for (uint8_t i = 0; i < 4; ++i) {
        for (uint8_t j = 0; j < 4; ++j) {
            (*state)[j][i] ^= RoundKey[(round * Nb * 4) + (i * Nb) + j];
        }
    }
}

void SubBytes(state_t* state) {
    for (uint8_t i = 0; i < 4; ++i) {
        for (uint8_t j = 0; j < 4; ++j) {
            (*state)[j][i] = s_box[(*state)[j][i]];
        }
    }
}


/*****************************************************************************/
/* AES OPERATIONS (STANDARD VS ACCELERATED)                                  */
/* This section is unchanged from the original.                              */
/*****************************************************************************/

#ifdef USE_RISCV_ACCEL
// ACCELERATED VERSION (using Zbb and Zbc extensions)
void ShiftRows(state_t* state) {
    uint32_t row1, row2, row3;
    row1 = ((*state)[1][0] << 24) | ((*state)[1][1] << 16) | ((*state)[1][2] << 8) | (*state)[1][3];
    asm volatile ("rori %0, %1, 24" : "=r"(row1) : "r"(row1));
    (*state)[1][0] = (row1 >> 24); (*state)[1][1] = (row1 >> 16); (*state)[1][2] = (row1 >> 8); (*state)[1][3] = row1;
    row2 = ((*state)[2][0] << 24) | ((*state)[2][1] << 16) | ((*state)[2][2] << 8) | (*state)[2][3];
    asm volatile ("rori %0, %1, 16" : "=r"(row2) : "r"(row2));
    (*state)[2][0] = (row2 >> 24); (*state)[2][1] = (row2 >> 16); (*state)[2][2] = (row2 >> 8); (*state)[2][3] = row2;
    row3 = ((*state)[3][0] << 24) | ((*state)[3][1] << 16) | ((*state)[3][2] << 8) | (*state)[3][3];
    asm volatile ("rori %0, %1, 8" : "=r"(row3) : "r"(row3));
    (*state)[3][0] = (row3 >> 24); (*state)[3][1] = (row3 >> 16); (*state)[3][2] = (row3 >> 8); (*state)[3][3] = row3;
}
void MixColumns(state_t* state) {
    uint32_t t, a, b;
    const uint32_t M2 = 0x02020202; const uint32_t M3 = 0x03030303;
    for (int i = 0; i < 4; i++) {
        a = (*state)[0][i] | (*state)[1][i] << 8 | (*state)[2][i] << 16 | (*state)[3][i] << 24;
        asm volatile ("clmul %0, %1, %2" : "=r"(t) : "r"(a), "r"(M2));
        asm volatile ("clmul %0, %1, %2" : "=r"(b) : "r"(a), "r"(M3));
        t ^= b; b = a;
        asm volatile ("rori %0, %1, 8" : "=r"(b) : "r"(b)); t ^= b;
        asm volatile ("rori %0, %1, 8" : "=r"(b) : "r"(b)); t ^= b;
        asm volatile ("rori %0, %1, 8" : "=r"(b) : "r"(b)); t ^= b;
        (*state)[0][i] = t; (*state)[1][i] = t >> 8;
        (*state)[2][i] = t >> 16; (*state)[3][i] = t >> 24;
    }
}
#else
// STANDARD C VERSION (baseline)
void ShiftRows(state_t* state) {
    uint8_t temp;
    temp = (*state)[1][0]; (*state)[1][0] = (*state)[1][1]; (*state)[1][1] = (*state)[1][2]; (*state)[1][2] = (*state)[1][3]; (*state)[1][3] = temp;
    temp = (*state)[2][0]; (*state)[2][0] = (*state)[2][2]; (*state)[2][2] = temp;
    temp = (*state)[2][1]; (*state)[2][1] = (*state)[2][3]; (*state)[2][3] = temp;
    temp = (*state)[3][0]; (*state)[3][0] = (*state)[3][3]; (*state)[3][3] = (*state)[3][2]; (*state)[3][2] = (*state)[3][1]; (*state)[3][1] = temp;
}
uint8_t xtime(uint8_t x) { return ((x << 1) ^ (((x >> 7) & 1) * 0x1b)); }
void MixColumns(state_t* state) {
    uint8_t i, a, b, c, d;
    for (i = 0; i < 4; ++i) {
        a = (*state)[0][i]; b = (*state)[1][i]; c = (*state)[2][i]; d = (*state)[3][i];
        (*state)[0][i] = xtime(a) ^ (xtime(b) ^ b) ^ c ^ d;
        (*state)[1][i] = a ^ xtime(b) ^ (xtime(c) ^ c) ^ d;
        (*state)[2][i] = a ^ b ^ xtime(c) ^ (xtime(d) ^ d);
        (*state)[3][i] = (xtime(a) ^ a) ^ b ^ c ^ xtime(d);
    }
}
#endif

/*****************************************************************************/
/* UNIFIED AES ENCRYPTION CIPHER                                             */
/*****************************************************************************/

void aes_encrypt(state_t* state, const uint8_t* RoundKey) {
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
}

// Helper to convert a 1D block buffer to the 2D state matrix
void block_to_state(const uint8_t* block, state_t* state) {
    for(int r = 0; r < 4; ++r) {
        for(int c = 0; c < 4; ++c) {
            (*state)[r][c] = block[c * 4 + r];
        }
    }
}

// Helper to convert the 2D state matrix back to a 1D block buffer
void state_to_block(const state_t* state, uint8_t* block) {
    for(int r = 0; r < 4; ++r) {
        for(int c = 0; c < 4; ++c) {
            block[c * 4 + r] = (*state)[r][c];
        }
    }
}

/*****************************************************************************/
/* NEW FILE HANDLING AND MAIN FUNCTION                                       */
/*****************************************************************************/

// Creates a file of a given size with pseudo-random data.
int create_test_file(const char *filename, long long size) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        perror("Failed to create test file");
        return -1;
    }
    // Use a buffer for efficiency
    const int buffer_size = 4096;
    uint8_t buffer[buffer_size];
    srand(time(NULL)); // Seed random number generator

    for (int i = 0; i < buffer_size; i++) {
        buffer[i] = rand() % 256;
    }

    long long bytes_written = 0;
    while(bytes_written < size) {
        long long to_write = size - bytes_written;
        if (to_write > buffer_size) {
            to_write = buffer_size;
        }
        if(fwrite(buffer, 1, to_write, fp) != to_write) {
            perror("Error writing to test file");
            fclose(fp);
            return -1;
        }
        bytes_written += to_write;
    }

    fclose(fp);
    printf("Successfully created test file '%s' of %lld bytes.\n", filename, size);
    return 0;
}


int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file_size_in_bytes>\n", argv[0]);
        fprintf(stderr, "Example: %s 10485760  (for a 10MB file)\n", argv[0]);
        return 1;
    }

    long long file_size = atoll(argv[1]);
    if (file_size <= 0) {
        fprintf(stderr, "Error: File size must be a positive number.\n");
        return 1;
    }

    const char* IN_FILENAME = "test_data.bin";
    const char* OUT_FILENAME = "test_data.enc";
    uint8_t key[] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
    uint8_t RoundKey[176];

    // 1. Generate the workload file
    if (create_test_file(IN_FILENAME, file_size) != 0) {
        return 1;
    }

    // 2. Prepare for encryption
    FILE *in_file = fopen(IN_FILENAME, "rb");
    FILE *out_file = fopen(OUT_FILENAME, "wb");
    if (!in_file || !out_file) {
        perror("Error opening files for encryption");
        return 1;
    }

    KeyExpansion(RoundKey, key);

    printf("\n--- RISC-V AES File Encryption Benchmark ---\n");
#ifdef USE_RISCV_ACCEL
    printf("Mode:           Accelerated (Zbb, Zbc)\n");
#else
    printf("Mode:           Standard C (Baseline)\n");
#endif
    printf("Processing:     %s -> %s\n", IN_FILENAME, OUT_FILENAME);
    printf("File Size:      %.2f MB\n", (double)file_size / (1024 * 1024));
    printf("Encrypting...\n");

    // 3. Encrypt the file block by block and time it
    clock_t start = clock();

    uint8_t in_block[AES_BLOCK_SIZE];
    uint8_t out_block[AES_BLOCK_SIZE];
    state_t state;
    size_t bytes_read;

    while ((bytes_read = fread(in_block, 1, AES_BLOCK_SIZE, in_file)) > 0) {
        // Handle padding for the last block
        if (bytes_read < AES_BLOCK_SIZE) {
            uint8_t pad_val = AES_BLOCK_SIZE - bytes_read;
            memset(in_block + bytes_read, pad_val, pad_val);
        }

        block_to_state(in_block, &state);
        aes_encrypt(&state, RoundKey);
        state_to_block(&state, out_block);

        fwrite(out_block, 1, AES_BLOCK_SIZE, out_file);
    }

    // PKCS#7: If the last block was perfectly full, add a new block of padding
    if (file_size > 0 && file_size % AES_BLOCK_SIZE == 0) {
        memset(in_block, AES_BLOCK_SIZE, AES_BLOCK_SIZE);
        block_to_state(in_block, &state);
        aes_encrypt(&state, RoundKey);
        state_to_block(&state, out_block);
        fwrite(out_block, 1, AES_BLOCK_SIZE, out_file);
    }

    clock_t end = clock();

    fclose(in_file);
    fclose(out_file);

    // 4. Report performance and clean up
    double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;
    double throughput_mbs = (double)file_size / (1024 * 1024) / time_taken;

    printf("\n--- Results ---\n");
    printf("Total Execution Time: %.4f seconds\n", time_taken);
    if(time_taken > 0) {
        printf("Throughput:           %.2f MB/s\n", throughput_mbs);
    }
    printf("---------------------------------\n");

    // Clean up temporary files
    remove(IN_FILENAME);
    remove(OUT_FILENAME);

    return 0;
}
