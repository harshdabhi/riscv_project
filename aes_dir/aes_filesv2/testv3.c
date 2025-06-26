/**
 * @file riscv_aes_eval_v2_sweep.c
 * @brief A C program for AES-128 that runs an automated benchmark sweep.
 *
 * This version automatically encrypts files of increasing size (10KB to 10MB)
 * and prints performance results for each step in a CSV format, suitable for
 * plotting and analysis.
 *
 * --- How to Compile and Run ---
 *
 * 1. Compile WITHOUT acceleration (baseline):
 * riscv64-unknown-elf-gcc -O2 -march=rv64g -o aes_standard_sweep riscv_aes_eval_v2_sweep.c
 *
 * 2. Compile WITH RISC-V acceleration:
 * riscv64-unknown-elf-gcc -O2 -march=rv64gc_zba_zbb_zbc_zbs -D USE_RISCV_ACCEL -o aes_accelerated_sweep riscv_aes_eval_v2_sweep.c
 *
 * 3. Run and Collect Data:
 * The program now runs the full sweep automatically.
 *
 * qemu-riscv64 aes_standard_sweep > standard_results.csv
 * qemu-riscv64 aes_accelerated_sweep > accelerated_results.csv
 *
 * 4. Analyze:
 * Open the generated .csv files in a spreadsheet program to plot throughput
 * vs. file size and compare the two versions.
 *
 * 5. For Resource/Energy Usage:
 * Use external tools like 'perf' on Linux or simulator-specific features
 * while the program is running. The C code cannot measure this itself.
 * Example: perf stat -e cycles,instructions ./qemu-riscv64 aes_accelerated_sweep
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Defines for AES-128
#define Nk 4
#define Nb 4
#define Nr 10
#define AES_BLOCK_SIZE 16

// --- Core AES Logic (S-box, Rcon, KeyExpansion, etc.) ---
// This entire section is identical to the previous version and is
// omitted here for brevity. You should copy it from your previous file.
// ... (paste the entire block of AES code from s_box to state_to_block here) ...

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
typedef uint8_t state_t[4][4];
void KeyExpansion(uint8_t* RoundKey, const uint8_t* Key) {
    uint32_t i, j, k;
    uint8_t tempa[4];
    for (i = 0; i < Nk; ++i) {
        RoundKey[(i * 4) + 0] = Key[(i * 4) + 0]; RoundKey[(i * 4) + 1] = Key[(i * 4) + 1];
        RoundKey[(i * 4) + 2] = Key[(i * 4) + 2]; RoundKey[(i * 4) + 3] = Key[(i * 4) + 3];
    }
    for (i = Nk; i < Nb * (Nr + 1); ++i) {
        k = (i - 1) * 4;
        tempa[0] = RoundKey[k + 0]; tempa[1] = RoundKey[k + 1]; tempa[2] = RoundKey[k + 2]; tempa[3] = RoundKey[k + 3];
        if (i % Nk == 0) {
            const uint8_t u8tmp = tempa[0];
            tempa[0] = tempa[1]; tempa[1] = tempa[2]; tempa[2] = tempa[3]; tempa[3] = u8tmp;
            tempa[0] = s_box[tempa[0]]; tempa[1] = s_box[tempa[1]]; tempa[2] = s_box[tempa[2]]; tempa[3] = s_box[tempa[3]];
            tempa[0] = tempa[0] ^ Rcon[i / Nk];
        }
        j = i * 4; k = (i - Nk) * 4;
        RoundKey[j + 0] = RoundKey[k + 0] ^ tempa[0]; RoundKey[j + 1] = RoundKey[k + 1] ^ tempa[1];
        RoundKey[j + 2] = RoundKey[k + 2] ^ tempa[2]; RoundKey[j + 3] = RoundKey[k + 3] ^ tempa[3];
    }
}
void AddRoundKey(uint8_t round, state_t* state, const uint8_t* RoundKey) {
    for (uint8_t i = 0; i < 4; ++i) for (uint8_t j = 0; j < 4; ++j) (*state)[j][i] ^= RoundKey[(round*Nb*4)+(i*Nb)+j];
}
void SubBytes(state_t* state) {
    for (uint8_t i=0;i<4;++i) for (uint8_t j=0;j<4;++j) (*state)[j][i] = s_box[(*state)[j][i]];
}
#ifdef USE_RISCV_ACCEL
void ShiftRows(state_t* state) {
    uint32_t r1,r2,r3;
    r1=((*state)[1][0]<<24)|((*state)[1][1]<<16)|((*state)[1][2]<<8)|(*state)[1][3]; asm volatile("rori %0,%1,24":"=r"(r1):"r"(r1));
    (*state)[1][0]=(r1>>24);(*state)[1][1]=(r1>>16);(*state)[1][2]=(r1>>8);(*state)[1][3]=r1;
    r2=((*state)[2][0]<<24)|((*state)[2][1]<<16)|((*state)[2][2]<<8)|(*state)[2][3]; asm volatile("rori %0,%1,16":"=r"(r2):"r"(r2));
    (*state)[2][0]=(r2>>24);(*state)[2][1]=(r2>>16);(*state)[2][2]=(r2>>8);(*state)[2][3]=r2;
    r3=((*state)[3][0]<<24)|((*state)[3][1]<<16)|((*state)[3][2]<<8)|(*state)[3][3]; asm volatile("rori %0,%1,8":"=r"(r3):"r"(r3));
    (*state)[3][0]=(r3>>24);(*state)[3][1]=(r3>>16);(*state)[3][2]=(r3>>8);(*state)[3][3]=r3;
}
void MixColumns(state_t* state) {
    uint32_t t,a,b; const uint32_t M2=0x02020202,M3=0x03030303;
    for(int i=0;i<4;i++){
        a=(*state)[0][i]|(*state)[1][i]<<8|(*state)[2][i]<<16|(*state)[3][i]<<24;
        asm volatile("clmul %0,%1,%2":"=r"(t):"r"(a),"r"(M2)); asm volatile("clmul %0,%1,%2":"=r"(b):"r"(a),"r"(M3));
        t^=b; b=a; asm volatile("rori %0,%1,8":"=r"(b):"r"(b)); t^=b;
        asm volatile("rori %0,%1,8":"=r"(b):"r"(b)); t^=b; asm volatile("rori %0,%1,8":"=r"(b):"r"(b)); t^=b;
        (*state)[0][i]=t;(*state)[1][i]=t>>8;(*state)[2][i]=t>>16;(*state)[3][i]=t>>24;
    }
}
#else
void ShiftRows(state_t* state){uint8_t t;t=(*state)[1][0];(*state)[1][0]=(*state)[1][1];(*state)[1][1]=(*state)[1][2];(*state)[1][2]=(*state)[1][3];(*state)[1][3]=t;t=(*state)[2][0];(*state)[2][0]=(*state)[2][2];(*state)[2][2]=t;t=(*state)[2][1];(*state)[2][1]=(*state)[2][3];(*state)[2][3]=t;t=(*state)[3][0];(*state)[3][0]=(*state)[3][3];(*state)[3][3]=(*state)[3][2];(*state)[3][2]=(*state)[3][1];(*state)[3][1]=t;}
uint8_t xtime(uint8_t x){return((x<<1)^(((x>>7)&1)*0x1b));}
void MixColumns(state_t* state){uint8_t i,a,b,c,d;for(i=0;i<4;++i){a=(*state)[0][i];b=(*state)[1][i];c=(*state)[2][i];d=(*state)[3][i];(*state)[0][i]=xtime(a)^(xtime(b)^b)^c^d;(*state)[1][i]=a^xtime(b)^(xtime(c)^c)^d;(*state)[2][i]=a^b^xtime(c)^(xtime(d)^d);(*state)[3][i]=(xtime(a)^a)^b^c^xtime(d);}}
#endif
void aes_encrypt(state_t* state, const uint8_t* RoundKey) {
    AddRoundKey(0, state, RoundKey);
    for (uint8_t round=1; round<Nr; ++round) {SubBytes(state); ShiftRows(state); MixColumns(state); AddRoundKey(round, state, RoundKey);}
    SubBytes(state); ShiftRows(state); AddRoundKey(Nr, state, RoundKey);
}
void block_to_state(const uint8_t* block, state_t* state){for(int r=0;r<4;++r)for(int c=0;c<4;++c)(*state)[r][c]=block[c*4+r];}
void state_to_block(const state_t* state, uint8_t* block){for(int r=0;r<4;++r)for(int c=0;c<4;++c)block[c*4+r]=(*state)[r][c];}


/*****************************************************************************/
/* AUTOMATED BENCHMARKING SUITE                                              */
/*****************************************************************************/

const char* IN_FILENAME = "temp_data.bin";
const char* OUT_FILENAME = "temp_data.enc";

// A struct to hold the results of a single benchmark run
typedef struct {
    double execution_time;
    double throughput_mbs;
} PerformanceResult;

// Creates a file of a given size with pseudo-random data.
int create_test_file(long long size) {
    FILE *fp = fopen(IN_FILENAME, "wb");
    if (!fp) return -1;
    for (long long i = 0; i < size; ++i) fputc(i % 256, fp);
    fclose(fp);
    return 0;
}

// Runs the encryption benchmark for a single file size
PerformanceResult run_benchmark_for_size(long long size, const uint8_t* RoundKey) {
    PerformanceResult result = {0.0, 0.0};

    if (create_test_file(size) != 0) {
        perror("Failed to create test file");
        return result;
    }

    FILE *in_file = fopen(IN_FILENAME, "rb");
    FILE *out_file = fopen(OUT_FILENAME, "wb");
    if (!in_file || !out_file) {
        perror("Error opening files for encryption");
        return result;
    }

    clock_t start = clock();

    uint8_t in_block[AES_BLOCK_SIZE];
    uint8_t out_block[AES_BLOCK_SIZE];
    state_t state;
    size_t bytes_read;

    while ((bytes_read = fread(in_block, 1, AES_BLOCK_SIZE, in_file)) > 0) {
        if (bytes_read < AES_BLOCK_SIZE) {
            uint8_t pad_val = AES_BLOCK_SIZE - bytes_read;
            memset(in_block + bytes_read, pad_val, pad_val);
        }
        block_to_state(in_block, &state);
        aes_encrypt(&state, RoundKey);
        state_to_block(&state, out_block);
        fwrite(out_block, 1, AES_BLOCK_SIZE, out_file);
    }

    if (size > 0 && size % AES_BLOCK_SIZE == 0) {
        memset(in_block, AES_BLOCK_SIZE, AES_BLOCK_SIZE);
        block_to_state(in_block, &state);
        aes_encrypt(&state, RoundKey);
        state_to_block(&state, out_block);
        fwrite(out_block, 1, AES_BLOCK_SIZE, out_file);
    }

    clock_t end = clock();

    fclose(in_file);
    fclose(out_file);
    remove(IN_FILENAME);
    remove(OUT_FILENAME);

    result.execution_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    if (result.execution_time > 0) {
        result.throughput_mbs = (double)size / (1024 * 1024) / result.execution_time;
    }

    return result;
}

int main() {
    // Benchmark parameters
    const long long START_SIZE = 10 * 1024;      // 10 KB
    const long long END_SIZE   = 10 * 1024 * 1024; // 10 MB
    const long long STEP_SIZE  = 10 * 1024;      // 10 KB

    uint8_t key[] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
    uint8_t RoundKey[176];

    // --- Setup ---
    printf("--- RISC-V AES Performance Sweep ---\n");
#ifdef USE_RISCV_ACCEL
    printf("Mode: Accelerated (Zbb, Zbc)\n");
#else
    printf("Mode: Standard C (Baseline)\n");
#endif
    printf("Workload: Encrypting files from %lld KB to %lld MB in %lld KB steps.\n\n",
           START_SIZE / 1024, END_SIZE / (1024 * 1024), STEP_SIZE / 1024);

    // Expand key once before the loop
    KeyExpansion(RoundKey, key);

    // --- Print CSV Header ---
    // This format is easy to paste into a spreadsheet for plotting.
    printf("%-15s, %-19s, %-18s\n", "File Size (KB)", "Execution Time (s)", "Throughput (MB/s)");
    printf("---------------, -------------------, ------------------\n");

    // --- Run Benchmark Sweep ---
    for (long long current_size = START_SIZE; current_size <= END_SIZE; current_size += STEP_SIZE) {
        PerformanceResult result = run_benchmark_for_size(current_size, RoundKey);

        // Print results for this step in CSV format
        printf("%-15lld, %-19.6f, %-18.2f\n",
               current_size / 1024,
               result.execution_time,
               result.throughput_mbs);
    }

    printf("\n--- Benchmark Sweep Complete ---\n");

    return 0;
}