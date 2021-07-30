ASM_SRC = $(wildcard src/asm/*.asm)
ASM_OBJS = $(ASM_SRC:.asm=.o)
C_SRC = $(wildcard src/*.c)
C_OBJS = $(C_SRC:.c=.o)

ISO_FILE = bin/JuiceOS.iso
OBJECTS = bin/build/objs
LINK = bin/linkers
BUILD = bin/build
ISO = bin/iso

build: cleanObjs $(ASM_OBJS) $(C_OBJS)
	@echo "LD $(ASM_OBJS) $(C_OBJS)"
	@ld -m elf_i386 -T $(LINK)/kernel.ld -o $(BUILD)/juiceos_k32.elf $(ASM_OBJS) $(C_OBJS)
	
	@rm -f src/asm/*.o
	@rm -f src/*.o
	@cp $(BUILD)/juiceos_k32.elf $(ISO)/

	@grub-mkrescue -V "JuiceOS" -o $(ISO_FILE) $(ISO)
	@echo "Build successfull!"

$(ASM_OBJS):
	@echo "NASM $(@:.o=.asm)"
	@nasm -f elf32 -o $@ $(@:.o=.asm)

$(C_OBJS):
	@echo "GCC $(@:.o=.c)"
	@gcc -m32 -o $@ -c $(@:.o=.c)

cleanObjs:
	@rm -f src/asm/*.o
	@rm -f src/*.o

clean:
	@rm -f $(BUILD)/*.elf
	@rm -f $(ISO)/*.elf
	@rm -f bin/*.iso

run:
	@qemu-system-x86_64 -cdrom $(ISO_FILE)