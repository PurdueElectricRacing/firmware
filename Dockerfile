FROM alpine:3.14.0

ENV APK_ADD="make cmake gcc-arm-none-eabi gdb-multiarch"
RUN apk update \
    && apk upgrade \
    && apk add --no-cache ${APK_ADD}

ENTRYPOINT [ "sh" ]