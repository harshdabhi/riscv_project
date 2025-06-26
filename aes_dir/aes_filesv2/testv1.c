/**
 * @file riscv_aes_eval_v2.c
 * @brief A C program for AES-128 that can be compiled WITH or WITHOUT
 * RISC-V crypto extensions for performance comparison.
 *
 * This version uses preprocessor macros to select code paths at compile time.
 * This allows for creating two distinct binaries from the same source file
 * for a true performance comparison.
 *
 * The accelerated path uses:
 * - 'rori' (Zbb) for the ShiftRows step.
 * - 'clmul' (Zbc) for the MixColumns step.
 *
 * --- How to Compile and Run ---
 *
 * 1. Compile WITHOUT acceleration (baseline):
 * The compiler will only use standard C operations.
 *
 * riscv64-unknown-elf-gcc -O2 -march=rv64g -o aes_standard riscv_aes_eval_v2.c
 *
 * 2. Compile WITH RISC-V acceleration:
 * We enable the extensions and define the USE_RISCV_ACCEL macro.
 *
 * riscv64-unknown-elf-gcc -O2 -march=rv64gc_zba_zbb_zbc_zbs -D USE_RISCV_ACCEL -o aes_accelerated riscv_aes_eval_v2.c
 *
 * 3. Run and Compare Timings:
 * Execute both binaries and compare their runtimes.
 *
 * qemu-riscv64 aes_standard
 * qemu-riscv64 aes_accelerated
 *
 * The 'aes_accelerated' binary should show a significant performance improvement.
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

// Defines for AES-128
#define Nk 4  // Number of 32-bit words in the key
#define Nb 4  // Number of columns (32-bit words) in the state
#define Nr 10 // Number of rounds for AES-128

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

void print_hex(const uint8_t* arr, int len) {
    for (int i = 0; i < len; ++i) printf("%02x ", arr[i]);
    printf("\n");
}

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
/*****************************************************************************/

#ifdef USE_RISCV_ACCEL

// ACCELERATED VERSION (using Zbb and Zbc extensions)

void ShiftRows(state_t* state) {
    // Pack rows into 32-bit words, rotate with 'rori', then unpack.
    uint32_t row1, row2, row3;

    // Row 1: Rotate left by 8 bits (rori amount 24)
    row1 = ((*state)[1][0] << 24) | ((*state)[1][1] << 16) | ((*state)[1][2] << 8) | (*state)[1][3];
    asm volatile ("rori %0, %1, 24" : "=r"(row1) : "r"(row1));
    (*state)[1][0] = (row1 >> 24); (*state)[1][1] = (row1 >> 16); (*state)[1][2] = (row1 >> 8); (*state)[1][3] = row1;

    // Row 2: Rotate left by 16 bits (rori amount 16)
    row2 = ((*state)[2][0] << 24) | ((*state)[2][1] << 16) | ((*state)[2][2] << 8) | (*state)[2][3];
    asm volatile ("rori %0, %1, 16" : "=r"(row2) : "r"(row2));
    (*state)[2][0] = (row2 >> 24); (*state)[2][1] = (row2 >> 16); (*state)[2][2] = (row2 >> 8); (*state)[2][3] = row2;

    // Row 3: Rotate left by 24 bits (rori amount 8)
    row3 = ((*state)[3][0] << 24) | ((*state)[3][1] << 16) | ((*state)[3][2] << 8) | (*state)[3][3];
    asm volatile ("rori %0, %1, 8" : "=r"(row3) : "r"(row3));
    (*state)[3][0] = (row3 >> 24); (*state)[3][1] = (row3 >> 16); (*state)[3][2] = (row3 >> 8); (*state)[3][3] = row3;
}

void MixColumns(state_t* state) {
    // This is a complex but highly optimized implementation using clmul.
    // It processes columns as 32-bit words.
    uint32_t t, a, b;
    const uint32_t M1 = 0x01010101;
    const uint32_t M2 = 0x02020202;
    const uint32_t M3 = 0x03030303;

    for (int i = 0; i < 4; i++) {
        // Pack state column into a word
        a = (*state)[0][i] | (*state)[1][i] << 8 | (*state)[2][i] << 16 | (*state)[3][i] << 24;

        // Carry-less multiply by 2 (for xtime)
        asm volatile ("clmul %0, %1, %2" : "=r"(t) : "r"(a), "r"(M2));

        // Carry-less multiply by 3 (for xtime(x)^x)
        asm volatile ("clmul %0, %1, %2" : "=r"(b) : "r"(a), "r"(M3));

        t ^= b;

        // Rotate and XOR
        b = a;
        asm volatile ("rori %0, %1, 8" : "=r"(b) : "r"(b));
        t ^= b;
        asm volatile ("rori %0, %1, 8" : "=r"(b) : "r"(b));
        t ^= b;
        asm volatile ("rori %0, %1, 8" : "=r"(b) : "r"(b));
        t ^= b;

        // Unpack word back into state column
        (*state)[0][i] = t;
        (*state)[1][i] = t >> 8;
        (*state)[2][i] = t >> 16;
        (*state)[3][i] = t >> 24;
    }
}


#else

// STANDARD C VERSION (baseline)

void ShiftRows(state_t* state) {
    uint8_t temp;
    // Row 1
    temp = (*state)[1][0]; (*state)[1][0] = (*state)[1][1]; (*state)[1][1] = (*state)[1][2];
    (*state)[1][2] = (*state)[1][3]; (*state)[1][3] = temp;
    // Row 2
    temp = (*state)[2][0]; (*state)[2][0] = (*state)[2][2]; (*state)[2][2] = temp;
    temp = (*state)[2][1]; (*state)[2][1] = (*state)[2][3]; (*state)[2][3] = temp;
    // Row 3
    temp = (*state)[3][0]; (*state)[3][0] = (*state)[3][3]; (*state)[3][3] = (*state)[3][2];
    (*state)[3][2] = (*state)[3][1]; (*state)[3][1] = temp;
}

uint8_t xtime(uint8_t x) {
    return ((x << 1) ^ (((x >> 7) & 1) * 0x1b));
}

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
    uint8_t round = 0;
    AddRoundKey(0, state, RoundKey);
    for (round = 1; round < Nr; ++round) {
        SubBytes(state);
        ShiftRows(state);
        MixColumns(state);
        AddRoundKey(round, state, RoundKey);
    }
    SubBytes(state);
    ShiftRows(state);
    AddRoundKey(Nr, state, RoundKey);
}

/*****************************************************************************/
/* MAIN FUNCTION                                                             */
/*****************************************************************************/

int main() {
    const long int ITERATIONS = 200000;
    uint8_t key[] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
    uint8_t in[]  = {0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d, 0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34};
    uint8_t out[16];
    uint8_t RoundKey[176];
    state_t state;

    KeyExpansion(RoundKey, key);

    printf("--- RISC-V AES Performance Test ---\n");
#ifdef USE_RISCV_ACCEL
    printf("Mode: Accelerated (Zbb, Zbc)\n");
#else
    printf("Mode: Standard C (Baseline)\n");
#endif
    printf("Running %ld iterations...\n", ITERATIONS);

    clock_t start = clock();
    for(long int i = 0; i < ITERATIONS; ++i) {
        for(int r = 0; r < 4; ++r) for(int c = 0; c < 4; ++c) state[r][c] = in[r + 4*c];
        aes_encrypt(&state, RoundKey);
    }
    clock_t end = clock();

    for(int r = 0; r < 4; ++r) for(int c = 0; c < 4; ++c) out[r + 4*c] = state[r][c];

    printf("\nPlaintext:  "); print_hex(in, 16);
    printf("Ciphertext: "); print_hex(out, 16);

    // NIST Known Answer Test (KAT)
    uint8_t expected[] = {0x39, 0x25, 0x84, 0x1d, 0x02, 0xdc, 0x09, 0xfb, 0xdc, 0x11, 0x85, 0x97, 0x19, 0x6a, 0x0b, 0x32};
    if (memcmp(out, expected, 16) == 0) {
        printf("Verification: SUCCESS! (Matches NIST KAT)\n");
    } else {
        printf("Verification: FAILED! Output does not match expected result.\n");
    }

    double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("\nTotal Execution Time: %.4f seconds\n", time_taken);
    printf("---------------------------------\n");

    return 0;
}
