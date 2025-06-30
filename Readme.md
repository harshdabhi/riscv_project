# AES & SHA Performance Testing on RISC-V

## Project Overview
This project benchmarks AES and SHA cryptographic performance on RISC-V, comparing the base RV32I instruction set, RV32I_ZBB (bit manipulation extension), RV32I_ZKSH (SHA crypto extension), and other relevant variants. It uses a prebuilt Docker image (Created and pushed by Harsh Mayank Dabhi) for a reproducible environment and provides scripts for easy testing and validation.

---

## Features
- **Automated benchmarking** for AES and SHA on RISC-V
- **Comparison** between RV32I, RV32I_ZBB, RV32I_ZKSH, and other extension variants
- **Prebuilt Docker image** for easy setup
- **Scripts** for installation validation and running tests
- **Performance metrics**: throughput, code size, instruction count, and more

---

## Quick Start
1. **Pull the Docker image:**
   ```bash
   docker pull phoenix67/finalyear_project:v2
   ```
2. **Run the container:**
   ```bash
   docker run -it -v $(pwd):/app phoenix67/finalyear_project:v2
   ```
3. **VSCode Remote Development:**
   - Use the "Remote - Containers" extension to open this folder inside the running container for a full-featured dev environment.

---

## Usage
After entering the container, use the following scripts:

### 1. Validate Installation
```bash
chmod +x test_exec.sh
./test_exec.sh
```
- Checks RISC-V toolchain and QEMU installation
- Lists available tool versions

### 2. Run AES Benchmarks
```bash
./aes_exec.sh
```
- Compiles and runs AES tests for different RISC-V versions (RV32I, RV32I_ZBB, etc.)
- Outputs performance metrics to `aes_metrics.csv`

### 3. Run SHA Benchmarks
```bash
chmod +x sha_exec.sh
./sha_exec.sh
```
- Compiles and runs SHA tests for different RISC-V versions (including RV32I, RV32I_ZBB, RV32I_ZKSH, and others)
- Uses source files from the `sha_files` directory

## Visualizing Results

You can visualize and analyze the benchmark results using the provided Jupyter notebook:

### 1. Launch Jupyter Notebook
Inside the running Docker container, start Jupyter:
```bash
jupyter notebook --ip=0.0.0.0 --port=8888 --allow-root
```
- Copy the URL/token from the terminal and open it in your browser.

### 2. Open the Notebook
- Open `visualisation.ipynb` in the Jupyter interface.

### 3. What You Can Do
- Load and plot data from `aes_metrics.csv`, SHA results, and other result files (including those generated from `sha_files`)
- Visualize throughput, execution time, and code size comparisons
- Generate bar charts, line plots, and summary tables
- Customize or extend the notebook for your own analysis

---

## Directory Structure
```
.
├── aes_dir/
│   └── aes_files/
│       └── aes_test.c
├── sha_dir/
│   └── (SHA C source files for benchmarking)
├── test_exec.sh
├── aes_exec.sh
├── sha_exec.sh
├── aes_metrics.csv (generated after running tests)
├── README.md
└── ...
```

---

## FAQ & Troubleshooting
**Q: The scripts fail with permission denied?**
A: Run `chmod +x *.sh` to make scripts executable.

**Q: How do I update the Docker image?**
A: Run `docker pull phoenix67/finalyear_project:v2` to fetch the latest version.

**Q: Where are the results?**
A: Performance metrics are saved in `aes_metrics.csv` after running `./aes_exec.sh`.

**Q: Can I run my own C files?**
A: Yes, place them in the appropriate directory and modify the scripts as needed.

--


## Resources
- [RISC-V GNU Toolchain Documentation](https://github.com/riscv/riscv-gnu-toolchain)
- [QEMU Documentation](https://www.qemu.org/)
- [RISC-V Official Website](https://riscv.org/)
- [RISC-V Extensions Article](https://research.redhat.com/blog/article/risc-v-extensions-whats-available-and-how-to-find-it/)
- [QEMU Extension Discussion](https://gitlab.com/qemu-project/qemu/-/issues/2245)
- [Bit Manipulation Article](https://fprox.substack.com/p/risc-v-scalar-bit-manipulation-extensions)
- [SHA512 RISC-V Example](https://github.com/riscv/riscv-crypto/blob/main/benchmarks/sha512/zscrypto_rv32/sha512.c)
- [Bitmanip Example](https://github.com/mjosaarinen/lwaes_isa/blob/master/bitmanip.c)
- [Important RISCV coding document](https://lists.riscv.org/g/dev-partners/attachment/43/0/riscv-crypto-spec-scalar-v0.9.3-DRAFT.pdf#page=40&zoom=100,0,0)

- [Coding example RISCV](https://github.com/lowRISC/opentitan/blob/master/sw/device/lib/crypto/impl/)


---
