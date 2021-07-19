BUILD = bin/build
TEMP_OS = bin/build/OS.temp
RAW_OS = bin/OS.bin

build: init assembly
	@cat $(BUILD)/boot.bin $(BUILD)/kernel.bin > $(TEMP_OS)
	@dd if=/dev/zero of=$(RAW_OS) bs=512 count=2880 status=none
	@dd if=$(TEMP_OS) of=$(RAW_OS) conv=notrunc status=none

	@echo "Build successfull!"

init:
	@mkdir -p bin/
	@mkdir -p bin/iso/
	@mkdir -p bin/build/

assembly:
	@nasm -f bin -o $(BUILD)/boot.bin src/boot.asm
	@nasm -f bin -o $(BUILD)/kernel.bin src/kernel.asm

run:
	@qemu-system-x86_64 $(RAW_OS)