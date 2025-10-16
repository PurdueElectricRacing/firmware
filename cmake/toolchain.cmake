
cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

if(STM32_FAMILY MATCHES "F7")
    set(TARGET_CPU "cortex-m7")
    set(FPU_FLAGS "-mfpu=fpv5-d16 -mfloat-abi=hard")
elseif(STM32_FAMILY MATCHES "L4" OR STM32_FAMILY MATCHES "G4" OR STM32_FAMILY MATCHES "F4")
    set(TARGET_CPU "cortex-m4")
    set(FPU_FLAGS "-mfpu=fpv4-sp-d16 -mfloat-abi=hard")
endif()
set(COMMON_FLAGS "-mthumb -mcpu=${TARGET_CPU} --specs=nosys.specs ${FPU_FLAGS}")

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR ${TARGET_CPU})

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(TOOLCHAIN_DIR "/usr/gcc-arm-none-eabi-10-2020-q4-major/bin/")
if (EXISTS ${TOOLCHAIN_DIR})

else()
    set(TOOLCHAIN_DIR "")
endif()

find_program(CMAKE_C_COMPILER   arm-none-eabi-gcc REQUIRED)
find_program(CMAKE_ASM_COMPILER arm-none-eabi-gcc REQUIRED)
find_program(CMAKE_CXX_COMPILER arm-none-eabi-g++)
find_program(CMAKE_SIZE_UTIL    arm-none-eabi-size)

set(C_CXX_FLAGS  "--specs=nano.specs -ffunction-sections -fdata-sections -ffreestanding")
set(CXX_FLAGS    "-fno-exceptions -fno-rtti -fno-threadsafe-statics")

set(CMAKE_C_FLAGS_INIT          "${COMMON_FLAGS} ${C_CXX_FLAGS}"              CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_INIT        "${COMMON_FLAGS} ${C_CXX_FLAGS} ${CXX_FLAGS}" CACHE STRING "" FORCE)
set(CMAKE_ASM_FLAGS_INIT        "${COMMON_FLAGS} -x assembler-with-cpp"       CACHE STRING "" FORCE)
set(CMAKE_EXE_LINKER_FLAGS_INIT "-Wl,--gc-sections"                           CACHE STRING "" FORCE)

set(CMAKE_C_FLAGS_DEBUG     "-Og -g -Werror" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_DEBUG   "-Og -g"         CACHE STRING "" FORCE)
set(CMAKE_C_FLAGS_RELEASE   "-Os -DNDEBUG"   CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_RELEASE "-Os -DNDEBUG"   CACHE STRING "" FORCE)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

set(CMAKE_EXPORT_COMPILE_COMMANDS             ON)
set(CMAKE_C_USE_RESPONSE_FILE_FOR_INCLUDES    ON)
set(CMAKE_C_USE_RESPONSE_FILE_FOR_LIBRARIES   ON)
set(CMAKE_C_USE_RESPONSE_FILE_FOR_OBJECTS     ON)
set(CMAKE_CXX_USE_RESPONSE_FILE_FOR_INCLUDES  ON)
set(CMAKE_CXX_USE_RESPONSE_FILE_FOR_LIBRARIES ON)
set(CMAKE_CXX_USE_RESPONSE_FILE_FOR_OBJECTS   ON)
set(CMAKE_NINJA_FORCE_RESPONSE_FILE           ON)