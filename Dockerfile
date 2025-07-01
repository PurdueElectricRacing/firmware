FROM ubuntu:25.04

RUN apt update

# Install dev dependencies
RUN apt -y install --no-install-recommends gcc-arm-none-eabi
RUN apt -y install --no-install-recommends ninja-build cmake
RUN apt -y install --no-install-recommends git ssh

# Setup python virtual environment
# This is required because some packages (python-can) don't get installed properly when running as root
RUN apt -y install --no-install-recommends python3 python3-venv
ENV VIRTUAL_ENV=/opt/venv
RUN python3 -m venv $VIRTUAL_ENV
ENV PATH="$VIRTUAL_ENV/bin:$PATH"
COPY ./requirements.txt /usr/requirements.txt
RUN python3 -m pip install --requirement /usr/requirements.txt