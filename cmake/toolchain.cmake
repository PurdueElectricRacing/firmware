
cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

set(CMAKE_SYSTEM_NAME Generic)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

find_program(CMAKE_C_COMPILER   arm-none-eabi-gcc REQUIRED)
find_program(CMAKE_ASM_COMPILER arm-none-eabi-gcc REQUIRED)
find_program(CMAKE_OBJCOPY      arm-none-eabi-objcopy REQUIRED)
find_program(CMAKE_SIZE_UTIL    arm-none-eabi-size REQUIRED)

set(COMMON_FLAGS "-mthumb --specs=nosys.specs -std=c23 -fanalyzer -Wno-analyzer-infinite-loop")
set(C_FLAGS      "--specs=nano.specs -ffunction-sections -fdata-sections -ffreestanding")
set(LINKER_FLAGS "-Wl,--gc-sections -Wl,--no-warn-rwx-segments")

set(CMAKE_C_FLAGS_INIT          "${COMMON_FLAGS} ${C_FLAGS}"                  CACHE STRING "" FORCE)
set(CMAKE_ASM_FLAGS_INIT        "${COMMON_FLAGS} -x assembler-with-cpp"       CACHE STRING "" FORCE)
set(CMAKE_EXE_LINKER_FLAGS_INIT "${LINKER_FLAGS}"                             CACHE STRING "" FORCE)

set(CMAKE_C_FLAGS_DEBUG     "-Og -g -Werror" CACHE STRING "" FORCE)
set(CMAKE_C_FLAGS_RELEASE   "-Os -DNDEBUG"   CACHE STRING "" FORCE)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

set(CMAKE_EXPORT_COMPILE_COMMANDS             ON)
set(CMAKE_C_USE_RESPONSE_FILE_FOR_INCLUDES    ON)
set(CMAKE_C_USE_RESPONSE_FILE_FOR_LIBRARIES   ON)
set(CMAKE_C_USE_RESPONSE_FILE_FOR_OBJECTS     ON)
set(CMAKE_NINJA_FORCE_RESPONSE_FILE           ON)