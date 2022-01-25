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

# Setup python virtual enviornment
# This is required because some packages (python-can) don't get installed properly when running as root
RUN apt -y install --no-install-recommends python3 python3.8-venv
ENV VIRTUAL_ENV=/opt/venv
RUN python3 -m venv $VIRTUAL_ENV
ENV PATH="$VIRTUAL_ENV/bin:$PATH"
COPY ./requirements.txt /usr/requirements.txt
RUN python3 -m pip install --requirement /usr/requirements.txt