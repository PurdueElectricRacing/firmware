FROM alpine:3.14.0

# Install dev depenincies
ENV APK_ADD="make cmake gdb-multiarch"
RUN apk update \
    && apk upgrade \
    && apk add --no-cache ${APK_ADD}

# Install arm-none-eabi-gcc
WORKDIR /usr/
RUN wget https://developer.arm.com/-/media/Files/downloads/gnu-rm/6-2017q2/gcc-arm-none-eabi-6-2017-q2-update-linux.tar.bz2 \
    && tar xvf gcc-arm-none-eabi-6-2017-q2-update-linux.tar.bz2 \
    && rm gcc-arm-none-eabi-6-2017-q2-update-linux.tar.bz2

ENV PATH $PATH:/usr/gcc-arm-none-eabi-6-2017-q2-update/bin
RUN apk add gcompat

ENTRYPOINT [ "sh" ]