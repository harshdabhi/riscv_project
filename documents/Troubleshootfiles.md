To enable and optimize for the RISC-V Cryptography Extensions (K extension) when using the `riscv64-linux-gnu-gcc` compiler, you need to use specific compiler flags. These flags ensure that the compiler is aware of the K extension and generates efficient code for it. Below are the relevant flags and steps:

---

### 1. **Enable the K Extension**
   The K extension must be explicitly enabled in the compiler. Use the `-march` flag to specify the architecture and include the `_zk` or `_zkn` subset (depending on the specific cryptographic extensions you are targeting).

   For example:
   ```bash
   riscv64-linux-gnu-gcc -march=rv64imafdc_zk -o my_program my_program.c
   ```

   - `rv64imafdc`: This specifies the base RISC-V architecture (64-bit, integer, multiplication, atomic, floating-point, double-precision, and compressed instructions).
   - `_zk`: This enables the standard scalar cryptography extensions, including SHA (SHA-256, SHA-512).

   If you are targeting specific subsets of the K extension (e.g., only SHA), you can use `_zks` (for SHA) or `_zkn` (for NIST suite, including AES and SHA).

   Example for SHA-only:
   ```bash
   riscv64-linux-gnu-gcc -march=rv64imafdc_zks -o my_program my_program.c
   ```

---

### 2. **Optimization Flags**
   Use optimization flags to ensure the compiler generates efficient code. Common optimization flags include:
   - `-O2` or `-O3`: General optimization levels.
   - `-funroll-loops`: Unroll loops for better performance (useful for cryptographic algorithms).
   - `-ffast-math`: Optimize floating-point operations (if applicable).

   Example:
   ```bash
   riscv64-linux-gnu-gcc -march=rv64imafdc_zk -O3 -funroll-loops -o my_program my_program.c
   ```

---

### 3. **Check for K Extension Support**
   Ensure that your `riscv64-linux-gnu-gcc` compiler supports the K extension. You can check the supported extensions by running:
   ```bash
   riscv64-linux-gnu-gcc --target-help
   ```
   Look for `-march` options related to `zk`, `zks`, or `zkn`.

---

### 4. **Verify Assembly Output**
   To ensure the compiler is generating the correct K extension instructions, you can inspect the assembly output. Use the `-S` flag to generate assembly code:
   ```bash
   riscv64-linux-gnu-gcc -march=rv64imafdc_zk -S my_program.c
   ```
   Look for instructions like `sha256sum0`, `sha256sum1`, `sha256sig0`, `sha256sig1`, etc., which are part of the K extension.

---

### 5. **Link-Time Optimization (LTO)**
   If your program is split across multiple source files, consider using Link-Time Optimization (LTO) to optimize across translation units:
   ```bash
   riscv64-linux-gnu-gcc -march=rv64imafdc_zk -O3 -flto -o my_program my_program.c
   ```

---

### 6. **Target-Specific Tuning**
   If you know the specific RISC-V core you are targeting (e.g., SiFive U74, T-Head C906), you can use the `-mtune` flag to optimize for that core:
   ```bash
   riscv64-linux-gnu-gcc -march=rv64imafdc_zk -mtune=sifive-u74 -O3 -o my_program my_program.c
   ```

---

### Example Command
Here’s a complete example command for compiling a program with SHA acceleration using the K extension:
```bash
riscv64-linux-gnu-gcc -march=rv64imafdc_zks -O3 -funroll-loops -flto -o sha_program sha_program.c
```

---

### 7. **Verify Hardware Support**
   Ensure that the RISC-V hardware you are running the code on actually supports the K extension. If the hardware does not support the K extension, the program will fail to execute or fall back to software emulation, which will not provide performance benefits.

   You can check the hardware capabilities by running:
   ```bash
   cat /proc/cpuinfo
   ```
   or using a tool like `riscv64-linux-gnu-readelf` to inspect the binary.

---

By using these flags and ensuring proper hardware and compiler support, you should be able to leverage the RISC-V K extension for improved SHA hashing performance.

May 2025

/opt/riscv/bin/riscv64-unknown-linux-gnu-gcc  -march=rv64id_zk -S sha_code_simple.c