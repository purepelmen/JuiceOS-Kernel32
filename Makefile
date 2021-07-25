BUILD = bin/build
TEMP_OS = bin/build/OS.temp
ISO_FILE = bin/JuiceOS.iso
ISO = bin/iso
LINK = src/linkers/

FILES_TO_LINK = $(BUILD)/multiboot_header.asm.elf $(BUILD)/kernel.asm.elf $(BUILD)/kernel.c.elf

build: init assembly
	@cp $(BUILD)/kernel.elf $(ISO)/boot
	@grub-mkrescue -o $(ISO_FILE) $(ISO)
	@echo "Build successfull!"

assembly:
	@nasm -f elf32 -o $(BUILD)/multiboot_header.asm.elf src/multiboot_header.asm
	@nasm -f elf32 -o $(BUILD)/kernel.asm.elf src/kernel.asm

	@gcc -m32 -o $(BUILD)/kernel.c.elf -c src/kernel.c

	@ld -m elf_i386 -T $(LINK)/kernel.ld -o $(BUILD)/kernel.elf $(FILES_TO_LINK)

init:
	@mkdir -p bin/
	@mkdir -p bin/iso/
	@mkdir -p bin/build/

run:
	@qemu-system-x86_64 $(RAW_OS)