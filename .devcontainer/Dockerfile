FROM ubuntu:22.04 AS env


# Update the package list and install the required packages
RUN apt-get update && \
    apt-get install -y \
    autoconf \
    automake \
    autotools-dev \
    curl \
    python3 \
    python3-pip \
    python3-tomli \
    libmpc-dev \
    libmpfr-dev \
    libgmp-dev \
    gawk \
    build-essential \
    bison \
    flex \
    texinfo \
    gperf \
    libtool \
    patchutils \
    bc \
    zlib1g-dev \
    libexpat-dev \
    ninja-build \
    git \
    cmake \
    libglib2.0-dev \
    libslirp-dev && \
    # Ensure pip is installed and upgraded
    python3 -m pip install --upgrade pip



FROM env AS toolchain
RUN apt update && apt install -y git && apt clean
RUN git clone --recursive https://github.com/riscv/riscv-gnu-toolchain
WORKDIR /riscv-gnu-toolchain
RUN ./configure --prefix=/opt/riscv && make linux -j `nproc`



FROM env AS qemu
RUN curl -O https://download.qemu.org/qemu-9.2.2.tar.xz && \
    tar -xvJf qemu-9.2.2.tar.xz
WORKDIR /qemu-9.2.2
RUN ./configure && make -j `nproc`

FROM env
COPY --from=toolchain /opt/riscv /opt/riscv
COPY --from=qemu /qemu-9.2.2/build/qemu-riscv64 /usr/local/bin/


RUN apt update && \
    apt install -y \
    gcc-riscv64-linux-gnu \
    g++-riscv64-linux-gnu \
    && apt clean


WORKDIR /root

