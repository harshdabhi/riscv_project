# RISC-V Project

This project demonstrates how to compile and run RISC-V C code using the RISC-V GNU toolchain and QEMU.

---

## Table of Contents

1. [Installation](#installation)
2. [Compiling RISC-V C Code](#compiling-risc-v-c-code)
3. [Running the Executable on QEMU](#running-the-executable-on-qemu)
4. [Important Resources](#important-resources)

---

## Installation

### Install RISC-V GNU Toolchain

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
- [QEMU Documentation](https://www.qemu.org/docs/)
- [RISC-V Official Website](https://riscv.org/)

---

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

---

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.


