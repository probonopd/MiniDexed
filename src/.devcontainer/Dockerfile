FROM mcr.microsoft.com/devcontainers/universal:linux

RUN mkdir -p /opt/gcc && \
    cd /opt/gcc && \
    wget -q https://developer.arm.com/-/media/Files/downloads/gnu-a/10.3-2021.07/binrel/gcc-arm-10.3-2021.07-x86_64-aarch64-none-elf.tar.xz && \
    tar xf gcc-arm-10.3-2021.07-x86_64-aarch64-none-elf.tar.xz && \
    rm gcc-arm-10.3-2021.07-x86_64-aarch64-none-elf.tar.xz && \
    wget -q https://developer.arm.com/-/media/Files/downloads/gnu-a/10.3-2021.07/binrel/gcc-arm-10.3-2021.07-x86_64-arm-none-eabi.tar.xz && \
    tar xf gcc-arm-10.3-2021.07-x86_64-arm-none-eabi.tar.xz && \
    rm gcc-arm-10.3-2021.07-x86_64-arm-none-eabi.tar.xz

ENV PATH=/opt/gcc/gcc-arm-10.3-2021.07-x86_64-aarch64-none-elf/bin:/opt/gcc/gcc-arm-10.3-2021.07-x86_64-arm-none-eabi/bin:$PATH
