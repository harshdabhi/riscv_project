Resource utilization generally refers to how efficiently various system resources (such as CPU, memory, disk I/O, etc.) are being used during the execution of a program. For the context of your SHA program, **CPU usage** and **memory usage** are two key resources to consider.

### **1. CPU Utilization**

CPU utilization measures how much of the CPU's processing power is used by the program. This is typically measured as a percentage of the total available CPU time.

To calculate CPU utilization, you can:
1. **Measure the total CPU time consumed by the program** during its execution.
2. **Compare it against the available CPU time** to estimate the percentage of CPU usage.

There are different ways to measure CPU utilization depending on the operating system you're using:

#### **On Linux/macOS (using `time` command)**

You can use the `time` command in the terminal to measure CPU usage. For example, to run your program and get resource usage:

```bash
time ./your_sha_program
```

This will give you output similar to:

```
real    0m0.215s
user    0m0.158s
sys     0m0.029s
```

- **real**: The wall-clock time (total elapsed time).
- **user**: The time the program spent executing in user mode.
- **sys**: The time the program spent executing in kernel mode.

From this output, you can get an idea of how much CPU time was consumed by your program (in the user and sys categories).

#### **On Linux/macOS (using `top` or `htop`)**

You can also use the `top` or `htop` command to monitor the CPU usage of your program in real-time. After running your program, use:

```bash
top -p <pid>
```

Where `<pid>` is the process ID of your program. `htop` provides a more user-friendly interface.

### **2. Memory Utilization**

Memory utilization refers to the amount of RAM your program uses during its execution. You can measure this in several ways, but common methods include:

#### **Using `getrusage()` in C**

In C, you can use the `getrusage()` function to get the memory usage and CPU time of your program. Here's how you can use it to measure the memory usage:

```c
#include <stdio.h>
#include <sys/resource.h>
#include <unistd.h>

void print_memory_usage() {
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);

    // Memory usage in kilobytes
    printf("Max memory usage: %ld KB\n", usage.ru_maxrss);
}

int main() {
    // Your SHA processing code

    // Print memory usage after execution
    print_memory_usage();

    return 0;
}
```

The `ru_maxrss` field in the `rusage` structure gives the maximum resident set size, i.e., the maximum amount of memory the process has used.

#### **Using `time` Command on Linux/macOS**

Just like CPU usage, you can also get memory utilization using the `time` command:

```bash
/usr/bin/time -v ./your_sha_program
```

The output will include:

```
Maximum resident set size (kbytes): 123456
```

This indicates the peak memory usage in kilobytes.

### **3. Disk I/O Utilization**

Disk I/O refers to the read and write operations your program performs on the disk. If you're dealing with large files (e.g., when hashing files), it’s important to measure disk I/O to see how much time is spent on reading or writing data.

To measure disk I/O, you can:
1. **Use `time` with the `-v` option** to get detailed I/O statistics.
2. **Use `iotop` (on Linux)** to monitor real-time disk I/O by specific processes.

Example with `time`:

```bash
/usr/bin/time -v ./your_sha_program
```

Look for lines like:

```
    Voluntary context switches: 2
    Involuntary context switches: 4
    Maximum resident set size (kbytes): 123456
    File system inputs: 0 bytes
    File system outputs: 0 bytes
```

### **4. Network Utilization (if applicable)**

If your program involves network operations (e.g., downloading files to hash), you should measure how much network bandwidth is used. On Linux, you can use tools like `iftop`, `nload`, or `netstat` to monitor network activity.

For example, you can use `iftop` to monitor the network usage of your program in real-time.

### **How to Calculate Resource Utilization**

To calculate resource utilization, you need to:
1. **Measure the resource consumption** (CPU, memory, disk, etc.) for a given period.
2. **Calculate the percentage or total amount** of resource usage during that period.

For example:

- **CPU Utilization**: From `time`, you might find that your program uses 0.2 seconds of CPU time out of 1 second of wall-clock time. CPU utilization would be:

  \[
  \text{CPU Utilization} = \frac{\text{CPU time}}{\text{Wall-clock time}} = \frac{0.2}{1} = 20\%
  \]

- **Memory Utilization**: If your program uses 50 MB of memory at its peak, that’s the maximum memory usage. You can compare this against the total available system memory to calculate the percentage of memory used.

  \[
  \text{Memory Utilization} = \frac{\text{Peak Memory Used}}{\text{Total Memory}} \times 100
  \]

### **Example Summary**

Let’s say you ran the `time` command on your program and got the following:

```
real    0m2.154s
user    0m1.795s
sys     0m0.359s
```

Additionally, `time -v` showed:

```
    Maximum resident set size (kbytes): 20480
    File system inputs: 0 bytes
    File system outputs: 0 bytes
```

- **CPU Utilization**: The user time is 1.795 seconds, and the wall-clock time is 2.154 seconds, so:

  \[
  \text{CPU Utilization} = \frac{1.795}{2.154} \times 100 \approx 83.3\%
  \]

- **Memory Utilization**: The maximum memory used is 20 MB (`20480` KB). You can compare this to your system's total memory. For example, if your system has 8 GB of RAM (8192 MB):

  \[
  \text{Memory Utilization} = \frac{20}{8192} \times 100 \approx 0.24\%
  \]

### **Additional Tools for Resource Utilization Measurement**

- **Linux**: `top`, `htop`, `time -v`, `iotop`, `getrusage()`, `ps`, `vmstat`
- **macOS**: `top`, `time -v`, `Activity Monitor`
- **Windows**: Task Manager, Resource Monitor, `Performance Monitor`

### **Summary**

1. **CPU Utilization**: Measure the time spent in user and system mode using `time` or `getrusage()` and calculate the percentage of total CPU usage.
2. **Memory Utilization**: Measure peak memory usage using `time -v` or `getrusage()`.
3. **Disk I/O Utilization**: Use `time -v` or tools like `iotop` to measure read/write operations.
4. **Network Utilization**: Use `iftop`, `netstat`, or `nload` if applicable.



Yes, it is possible to calculate energy efficiency using resource utilization (e.g., CPU usage, RAM usage) versus time. To make this calculation meaningful, you would need to understand the relationship between resource consumption and energy usage in your system, as well as how time factors into this calculation.

### General Concept:

Energy efficiency can be calculated by comparing the useful output (or work) to the total energy consumed (or resources used). In the case of computing, we often look at the energy consumption in terms of resource utilization (CPU, RAM, etc.) and time.

## Steps to Calculate Energy Efficiency:

1. **Determine Energy Consumption**:
   - Energy consumption can be estimated based on the usage of system resources (like CPU or RAM).
   - CPU and RAM utilization can be considered as proxies for energy consumption, although this is a rough estimate. For a more accurate calculation, you would typically need to have direct data about the power consumed by the system (such as watts).

2. **Measure Resource Utilization Over Time**:
   - Resource usage can be tracked over time, such as CPU utilization (%) or RAM utilization (%).
   - You would measure these over a period and integrate them over time to estimate total resource consumption.

3. **Energy Efficiency**:
   - Energy efficiency is often defined as the useful work (or task) done per unit of energy consumed. In a computational context, this could be the throughput (e.g., data processed, tasks completed) divided by the energy or resource consumption.

   - Energy Efficiency formula:
     \[
     \text{Energy Efficiency} = \frac{\text{Useful Work Done}}{\text{Energy Consumption}}
     \]
     where:
     - Useful work done can be things like throughput or completed tasks.
     - Energy consumption could be represented by resource utilization (CPU time, RAM usage) and time.

### Example Approach Using CPU Time and Resource Utilization:

Assume you measure:
- CPU time (`T_cpu`) used during a process.
- Total RAM used (`T_ram`), or some other relevant resource.
- Time taken for the computation (`T_time`).

Now, you can compute energy efficiency by dividing the total "work done" by the resources consumed.

1. **CPU Energy Efficiency**:
   \[
   \text{CPU Energy Efficiency} = \frac{\text{Throughput}}{\text{CPU Time} \times \text{CPU Utilization}}
   \]

2. **RAM Energy Efficiency**:
   \[
   \text{RAM Energy Efficiency} = \frac{\text{Throughput}}{\text{RAM Usage} \times \text{RAM Utilization}}
   \]

### Breakdown of the Code:
1. **`get_resource_utilization`**: Tracks the CPU time used and RAM utilization by the program.
2. **`calculate_throughput`**: Computes throughput as the file size divided by the CPU time.
3. **`calculate_energy_efficiency`**: Computes energy efficiency by dividing throughput by resource consumption (CPU and RAM).

### Energy Efficiency in Practice:
- This approach provides a rough estimate of energy efficiency based on resource utilization and time.
- In a real-world scenario, you would need to measure actual energy consumption (in watts), which would typically require hardware-level monitoring tools or power meters. However, using resource utilization as a proxy (CPU and RAM usage) allows us to estimate efficiency in software.

### How to Improve:
- If you have access to hardware power consumption data (like a power meter or energy profiler), you could replace resource utilization with actual energy consumption values.
- For a more accurate energy efficiency calculation, tools like Intel Power Gadget (for Intel CPUs) or software solutions for monitoring power draw can be used to get real-time power consumption statistics.

This provides a conceptual method for calculating energy efficiency based on the computational resources used and the time taken to complete a task. Let me know if you need further clarification or additional details