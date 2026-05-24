#!/bin/sh

# exit immediately if any command fails
set -e

# cleanup the test binary on exit
trap 'rm -f can_codec_tests' EXIT

# compile with no optimizations
cc -std=c23 -O0 -Wall -Wextra -I../.. can_codec_tests.c -o can_codec_tests

# run the test
./can_codec_tests
