FROM ubuntu:20.04

RUN apt update \
    && apt -y install wget

# Install arm-none-eabi-gcc by downloading from ARM, un-tar, 
# removing archive file, and adding the extracted folder to our path
WORKDIR /usr/
RUN wget https://developer.arm.com/-/media/Files/downloads/gnu-rm/10-2020q4/gcc-arm-none-eabi-10-2020-q4-major-x86_64-linux.tar.bz2 \
    && tar xvf gcc-arm-none-eabi-10-2020-q4-major-x86_64-linux.tar.bz2 \
    && rm gcc-arm-none-eabi-10-2020-q4-major-x86_64-linux.tar.bz2

ENV PATH $PATH:/usr/gcc-arm-none-eabi-10-2020-q4-major/bin

# Install dev depenincies
ENV APT_ADD="ninja-build cmake dos2unix"
RUN apt -y install --no-install-recommends ${APT_ADD}

RUN apt -y install --no-install-recommends python3

COPY bash.bashrc /etc/bash.bashrc
RUN dos2unix /etc/bash.bashrc