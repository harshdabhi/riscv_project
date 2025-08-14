# Choosing the Distro as Ubuntu
FROM ubuntu:22.04 AS env


# The required packages and dependencies
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
    libslirp-dev \
    libssl-dev\
    libncurses-dev \
    libz-dev &&\
    # Ensure pip is installed and upgraded
    python3 -m pip install --upgrade pip \
    && apt clean


# Installation of Gnu Toolchain
FROM env AS toolchain
RUN apt update && apt install -y git && apt clean
RUN git clone https://github.com/riscv/riscv-gnu-toolchain /riscv-gnu-toolchain
WORKDIR /riscv-gnu-toolchain
RUN ./configure --prefix=/opt/riscv --enable-multilib && make linux -j4


# Installation of Qemu latest version in case of further update please change the version
FROM env AS qemu
RUN curl -O https://download.qemu.org/qemu-9.2.2.tar.xz && \
    tar -xvJf qemu-9.2.2.tar.xz
WORKDIR /qemu-9.2.2
RUN ./configure --target-list=riscv32-linux-user,riscv64-linux-user && make -j4


# Moving the files from env to root
FROM env
COPY --from=toolchain /opt/riscv /opt/riscv
COPY --from=qemu /qemu-9.2.2/build/qemu-riscv64 /usr/local/bin/
COPY --from=qemu /qemu-9.2.2/build/qemu-riscv32 /usr/local/bin/



WORKDIR /root

