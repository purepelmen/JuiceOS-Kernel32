cmake_minimum_required(VERSION 3.10)

enable_language(ASM_NASM)
project(K32_Core)

file(GLOB_RECURSE SOURCES "*.cpp")
file(GLOB_RECURSE HEADERS "*.hpp")
file(GLOB_RECURSE ASMS "*.asm")

add_library(K32_Core STATIC ${SOURCES} ${HEADERS} ${ASMS})
target_include_directories(K32_Core PRIVATE ".")

target_compile_options(K32_Core PRIVATE $<$<COMPILE_LANGUAGE:ASM_NASM>:-f elf32>)
target_compile_options(K32_Core PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-nostdlib -nostdinc -ffreestanding -m32 -fno-rtti -fno-stack-protector>)
