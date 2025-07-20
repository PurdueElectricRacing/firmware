#!/usr/bin/env bash

docker buildx build --platform linux/amd64 -t eiln/per-firmware .
docker push eiln/per-firmware
docker buildx imagetools inspect eiln/per-firmware
