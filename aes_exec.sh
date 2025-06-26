#!/bin/bash

#<<<================================================================================================================================================================>>#

echo "accelerated AES encryption test begins"
/opt/riscv/bin/riscv64-unknown-linux-gnu-gcc -march=rv32imac_zbb_zbc -mabi=ilp32 -D USE_RISCV_ACCEL aes_dir/aes_filesv2/testv4.c -static -o aes_acc
/usr/local/bin/qemu-riscv32 ./aes_acc

#<<<================================================================================================================================================================>>#

echo "Moving files from source to AES result directory"
mv accelerated_results_aes.csv aes_result
mv aes_acc aes_result

#<<<================================================================================================================================================================>>#

echo "Removing files generated during test"
rm temp_data.bin
rm temp_data.enc

#<<<================================================================================================================================================================>>#

echo "Standard AES encryption test begins"
/opt/riscv/bin/riscv64-unknown-linux-gnu-gcc -march=rv32i -mabi=ilp32 aes_dir/aes_filesv2/testv4.c -static -o aes
/usr/local/bin/qemu-riscv32 ./aes

#<<<================================================================================================================================================================>>#

echo "Removing files generated during test"
rm temp_data.bin
rm temp_data.enc

#<<<================================================================================================================================================================>>#

echo "Moving files from source to AES result directory"
mv standard_results_aes.csv aes_result
mv aes aes_result


#<<<================================================================================================================================================================>>#
