cmake_minimum_required(VERSION 3.10)
set(CMAKE_ASM_NASM_OBJECT_FORMAT elf32)

project(K32_Core)
enable_language(ASM_NASM)

file(GLOB_RECURSE SOURCES "*.cpp")
file(GLOB_RECURSE HEADERS "*.h")
file(GLOB_RECURSE ASMS "*.asm")

add_library(K32_Core STATIC ${SOURCES} ${HEADERS} ${ASMS})
target_include_directories(K32_Core PRIVATE ".")

target_compile_options(K32_Core PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-nostdlib -nostdinc -ffreestanding -m32 -march=i386 -fno-rtti -fno-stack-protector --no-exceptions>)
