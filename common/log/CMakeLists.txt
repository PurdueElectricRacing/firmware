cmake_minimum_required(VERSION 3.13)

add_library(log INTERFACE)

# Find directories for '#include', this directroy for the simple lib
target_include_directories(log INTERFACE .)