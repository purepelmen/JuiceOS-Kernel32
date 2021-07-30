LINK_FILES = $(BUILD)/kernel_launcher.elf $(BUILD)/kernel.elf

ISO_FILE = bin/JuiceOS.iso
LINK = src/linkers/
BUILD = bin/build
ISO = bin/iso

build: assembly compileC
	@cp $(BUILD)/juiceos_k32.elf $(ISO)/
	@grub-mkrescue -V "JuiceOS" -o $(ISO_FILE) $(ISO)
	@echo "Build successfull!"

assembly:
	@nasm -f elf32 -o $(BUILD)/kernel_launcher.elf src/kernel_launcher.asm

compileC:
	@gcc -m32 -o $(BUILD)/kernel.elf -c src/kernel.c

	@ld -m elf_i386 -T $(LINK)/kernel.ld -o $(BUILD)/juiceos_k32.elf $(LINK_FILES)

clean:
	@rm -f $(BUILD)/*.elf
	@rm -f $(ISO)/boot/*.elf
	@rm -f bin/*.iso

run:
	@qemu-system-x86_64 -cdrom $(ISO_FILE)