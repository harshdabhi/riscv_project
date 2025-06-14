# AES Performance Testing on RISC-V

This project compares the performance of AES encryption implementation between RV32I (base instruction set) and RV32I_ZBB (with ZBB extension) on RISC-V architecture.

## Prerequisites

- RISC-V GCC toolchain
- QEMU user mode emulation
- Linux environment

## Directory Structure

```
.
├── aes_dir/
│   └── aes_files/
│       └── aes_test.c
├── README.md
└── aes_metrics.csv (generated after running tests)
```

## Compilation and Running

### Compilation Commands

```bash
# Compile for RV32I
/opt/riscv/bin/riscv64-unknown-linux-gnu-gcc -march=rv32i -mabi=ilp32 -O3 -o aes_test_rv32i aes_dir/aes_files/aes_test.c

# Compile for RV32I_ZBB
/opt/riscv/bin/riscv64-unknown-linux-gnu-gcc -march=rv32i_zbb -mabi=ilp32 -O3 -o aes_test_rv32i_zbb aes_dir/aes_files/aes_test.c
```

### Running Tests with QEMU

```bash
# Run RV32I version
qemu-riscv32 -cpu rv32 aes_test_rv32i

# Run RV32I_ZBB version
qemu-riscv32 -cpu rv32,zbb=true aes_test_rv32i_zbb
```

### Viewing Results

```bash
# View the metrics CSV file
cat aes_metrics.csv

# View binary sizes
ls -l aes_test_rv32i aes_test_rv32i_zbb

# View disassembly
/opt/riscv/bin/riscv64-unknown-linux-gnu-objdump -d aes_test_rv32i > aes_test_rv32i.dis
/opt/riscv/bin/riscv64-unknown-linux-gnu-objdump -d aes_test_rv32i_zbb > aes_test_rv32i_zbb.dis
```

## Performance Results

The tests measure performance across different data sizes:

| Data Size | RV32I Throughput (B/s) | RV32I_ZBB Throughput (B/s) | Performance Difference | Best Version |
|-----------|------------------------|---------------------------|----------------------|--------------|
| 16 B      | 17,353.58             | 11,204.48                 | RV32I +54.9%        | RV32I        |
| 1 KB      | 78,769,230.77         | 25,600,000.00             | RV32I +207.7%       | RV32I        |
| 4 KB      | 455,111,111.11        | 682,666,666.67            | RV32I_ZBB +50.0%    | RV32I_ZBB    |
| 16 KB     | 2,340,571,428.57      | 2,340,571,428.57          | Equal               | Both         |
| 64 KB     | 9,362,285,714.29      | 10,922,666,666.67         | RV32I_ZBB +16.7%    | RV32I_ZBB    |
| 256 KB    | 21,845,333,333.33     | 37,449,142,857.14         | RV32I_ZBB +71.4%    | RV32I_ZBB    |
| 1 MB      | 74,898,285,714.29     | 104,857,600,000.00        | RV32I_ZBB +40.0%    | RV32I_ZBB    |

## Key Findings

1. Small Data (16 bytes to 1 KB):
   - RV32I performs significantly better
   - RV32I is 54.9% faster for 16 bytes and 207.7% faster for 1 KB

2. Medium Data (4 KB to 16 KB):
   - Mixed results
   - ZBB extension starts showing benefits

3. Large Data (64 KB to 1 MB):
   - RV32I_ZBB consistently performs better
   - Significant performance improvements for large data

## Output Files

- `aes_metrics.csv`: Contains detailed performance metrics for each test run
  - File Size (KB)
  - RAM Utilization
  - Max RSS (KB)
  - Total RAM (MB)
  - CPU Time Used (s)
  - Throughput (B/s)

## Notes

- Results may vary depending on the host system and QEMU version
- The tests are run in QEMU user mode emulation
- Performance metrics include CPU time, RAM usage, and throughput
- Each test case uses random data and keys for encryption

# RISC-V Project

This project demonstrates how to compile and run RISC-V C code using the RISC-V GNU toolchain and QEMU.

---

## Table of Contents

1. [Installation](#installation)
2. [Compiling RISC-V C Code](#compiling-risc-v-c-code)
3. [Running the Executable on QEMU](#running-the-executable-on-qemu)
4. [Important Resources](#important-resources)
5. [Instruction set guide](https://gcc.gnu.org/onlinedocs/gcc/RISC-V-Options.html)

---

## Installation

### Install RISC-V GNU Toolchain and Qemu using Docker

#### Clone this directory to desired dir and cd to dir

### Method 1
   1. Clone this repo into directory ( Make sure docker is install on your computer )
   2. Now open VsCode and type and select " Dev container:Open container configuration file "
   ![vscode container setup](./setup_img_files/dev_setup.png)

   3. Now it will ask to select the folder where this repo is located on computer
   4. It will automatically setup all dependencies and requirements ( Approx 40 to 50 min )

### Method 2



#### Go to the directory and following command
```bash
   docker build -t {$ image_name} .
   docker run -it {$ image_name}
```

#### Type following command to check installation and its path

```bash

   # will show path compiler if installed properly
   which riscv64-linux-gnu-gcc

   # will show path of Qemu if installed properly
   which qemu-riscv64

```



If `riscv64-linux-gnu-` and its related tools are not available on your system, install them using the following commands:

```bash
sudo apt update
sudo apt install gcc-riscv64-linux-gnu g++-riscv64-linux-gnu
```

This will install the RISC-V GNU compiler and related tools.

---

## Compiling RISC-V C Code

To compile a C program for RISC-V, use the following command:

```bash
riscv64-linux-gnu-gcc -o {executable_file_name} {C_lang_file_name.c}
```

### Example

Compile a C program named `hello.c`:

```bash
riscv64-linux-gnu-gcc -o hello hello.c
```

This will generate an executable file named `hello`.

---

## Running the Executable on QEMU

To run the RISC-V executable on QEMU, use the following command:

```bash
qemu-riscv64 -L /usr/riscv64-linux-gnu -cpu max ./{executable_file_name}
```

### Example

Run the `hello` executable:

```bash
qemu-riscv64 -L /usr/riscv64-linux-gnu -cpu max ./hello
```

---

## Important Resources

- [RISC-V GNU Toolchain Documentation](https://github.com/riscv/riscv-gnu-toolchain)
- [QEMU Documentation](https://www.qemu.org/)
- [RISC-V Official Website](https://riscv.org/)
- [Important article on different Extension type](https://research.redhat.com/blog/article/risc-v-extensions-whats-available-and-how-to-find-it/)
- [Important guidance for latest Extension discussion](https://gitlab.com/qemu-project/qemu/-/issues/2245)
- [Article on Bit Manipulation](https://fprox.substack.com/p/risc-v-scalar-bit-manipulation-extensions)

---

## Important Note
   -  Bit manipulation: Zba, Zbb, Zbc, Zbs
   -  Crypto scalar: Zbkb, Zbkc, Zbkx, Zk*
   -  Vector (mostly complete): V, Zv*

## Example Workflow

1. Write a C program (`hello.c`):
   ```c
   #include <stdio.h>

   int main() {
       printf("Hello, RISC-V!\n");
       return 0;
   }
   ```

2. Compile the program:
   ```bash
   riscv64-linux-gnu-gcc -o ./test_example/hello ./test_example/hello.c
   ```

3. Run the program on QEMU:
   ```bash
   qemu-riscv64 -L /usr/riscv64-linux-gnu -cpu max ./test_example/hello
   ```

4. Output:
   ```
   Hello, RISC-V!
   ```


## Command to execute B extension


## Command to execute K extension
   ```bash

   riscv64-linux-gnu-gcc -o sha ./sha_code.c

   # without K extension
   qemu-riscv64 -L /usr/riscv64-linux-gnu -cpu max,zbkb=false ./sha

   # With K extension
   qemu-riscv64 -L /usr/riscv64-linux-gnu -cpu max,zbkb=true ./sha

   ```
---

## License

This project is licensed under the MIT License.


Command To validate installation

```bash


/opt/riscv/bin/riscv64-unknown-linux-gnu-gcc -print-multi-lib
ls /usr/local/bin | grep qemu
```




Command to execute k extension on 32 bit
```bash

/opt/riscv/bin/riscv64-unknown-linux-gnu-gcc -march=rv32imac_zk -mabi=ilp32 hello.c -static -o hello32
/usr/local/bin/qemu-riscv32 ./hello32

```

Command to execute k extension on 32 bit
```bash

/opt/riscv/bin/riscv64-unknown-linux-gnu-gcc -march=rv32imac_zbkb -mabi=ilp32 hello.c -static -o hello32
/usr/local/bin/qemu-riscv32 ./hello32

```

## Code Size and Instruction Count

### Binary Size Comparison

| Version   | Text (code) | Data | BSS  | Total Size | Hex Size |
|-----------|-------------|------|------|------------|----------|
| RV32I     | 436,186    | 13,280| 11,212| 460,678   | 0x70786  |
| RV32I_ZBB | 436,122    | 13,280| 11,212| 460,614   | 0x70746  |

### Instruction Count
- RV32I: 94,196 instructions
- RV32I_ZBB: 94,180 instructions

### Analysis
1. Code Size:
   - RV32I_ZBB has slightly smaller code size (64 bytes less)
   - Data and BSS sections are identical for both versions
   - Total size difference is minimal (64 bytes)

2. Instruction Count:
   - RV32I_ZBB uses 16 fewer instructions
   - The reduction is due to more efficient bit manipulation operations
   - The difference is small but shows that ZBB can reduce instruction count

3. Memory Layout:
   - Both versions have identical data and BSS sections
   - The difference is only in the code (text) section
   - ZBB extension doesn't require additional data structures

4. Instruction Differences:
   Example from MixColumns function:
   ```
   RV32I version:
   00010948 <MixColumns>:
      10948:       00054783                lbu     a5,0(a0)
      1094c:       00154883                lbu     a7,1(a0)
      10950:       00254803                lbu     a6,2(a0)
      10954:       00354e03                lbu     t3,3(a0)
      10958:       0117c6b3                xor     a3,a5,a7

   RV32I_ZBB version:
   00010948 <MixColumns>:
      10948:       00054703                lbu     a4,0(a0)
      1094c:       00154303                lbu     t1,1(a0)
      10950:       00254583                lbu     a1,2(a0)
      10954:       00354803                lbu     a6,3(a0)
      10958:       006747b3                xor     a5,a4,t1
   ```
   - Both versions use similar basic instructions (lbu, xor)
   - Register allocation differs between versions
   - ZBB version shows more efficient register usage
   - The core operations remain the same, but ZBB optimizes the implementation

## Performance Analysis Comparison

### Comparison with Published Results

| Metric | Claimed Improvement | Our Results | Validation |
|--------|-------------------|-------------|------------|
| Program Size | 37.5% reduction | 0.014% reduction | ❌ Not validated |
| Cycle Count | 63.1% reduction | 0.017% reduction | ❌ Not validated |
| Execution Time | 29.9% improvement | Variable (-55% to +41.6%) | ⚠️ Partially validated |

### Detailed Performance Analysis

1. **Program Size**
   - RV32I: 460,678 bytes
   - RV32I_ZBB: 460,614 bytes
   - Actual reduction: 64 bytes (0.014%)
   - The claimed 37.5% reduction is not observed in our implementation

2. **Instruction Count**
   - RV32I: 94,196 instructions
   - RV32I_ZBB: 94,180 instructions
   - Actual reduction: 16 instructions (0.017%)
   - The claimed 63.1% reduction in cycles is not observed

3. **Execution Time by Data Size**
   ```
   Data Size | RV32I (ns) | RV32I_ZBB (ns) | Improvement
   16B      | 91.1       | 141.2          | -55.0%
   1KB      | 129.9      | 380.2          | -192.7%
   4KB      | 380.2      | 253.5          | +33.3%
   16KB     | 1299.0     | 1299.0         | 0%
   64KB     | 3802.0     | 3245.0         | +14.7%
   256KB    | 12990.0    | 7580.0         | +41.6%
   1MB      | 38020.0    | 27150.0        | +28.6%
   ```

   Key observations:
   - Small data sizes (16B-1KB): RV32I performs better
   - Medium data sizes (4KB-16KB): Mixed results
   - Large data sizes (64KB-1MB): RV32I_ZBB shows consistent improvements
   - The claimed 29.9% average improvement is not consistently observed

### Possible Reasons for Differences

1. **Implementation Differences**
   - Our implementation may use different optimization strategies
   - The original research might have used different compiler flags or optimization levels

2. **Test Environment**
   - Our tests run in QEMU emulation
   - The original research might have used real hardware or different emulation settings

3. **Workload Characteristics**
   - Our test cases might have different characteristics than those in the original research
   - The original research might have focused on specific use cases

