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
RUN apt -y install --no-install-recommends ninja-build cmake
RUN apt -y install --no-install-recommends git ssh

# Setup python enviornment
RUN apt -y install --no-install-recommends python3 python3-pip
RUN apt -y install  python3-can
RUN pip3 install --upgrade pip
COPY ./requirements.txt /usr/requirements.txt
RUN pip3 install --requirement /usr/requirements.txt