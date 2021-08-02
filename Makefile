ASM_SRC   = $(wildcard src/asm/*.asm)
ASM_OBJS  = $(ASM_SRC:.asm=.o)
C_SRC	  = $(wildcard src/*.c)
C_OBJS	  = $(C_SRC:.c=.o)

ISO_FILE  = bin/JuiceOS.iso
LINK 	  = bin/linkers
BUILD 	  = bin/build
OBJECTS	  = bin/build/objects
ISO 	  = bin/iso

build: cleanObjects $(ASM_OBJS) $(C_OBJS)
	@echo "LD $(ASM_OBJS:src/asm%=$(OBJECTS)%) $(C_OBJS:src%=$(OBJECTS)%)"
	@ld -m elf_i386 -T $(LINK)/kernel.ld -o $(BUILD)/juiceos_k32.elf $(ASM_OBJS:src/asm%=$(OBJECTS)%) $(C_OBJS:src%=$(OBJECTS)%)
	
	@cp $(BUILD)/juiceos_k32.elf $(ISO)/

	@grub-mkrescue -V "JuiceOS" -o $(ISO_FILE) $(ISO)
	@echo "Build successfull!"

$(ASM_OBJS):
	@echo "NASM $(@:.o=.asm)"
	@nasm -f elf32 -o $(@:src/asm%=$(OBJECTS)%) $(@:.o=.asm)

$(C_OBJS):
	@echo "GCC $(@:.o=.c)"
	@gcc -ffreestanding -m32 -o $(@:src%=$(OBJECTS)%) -c $(@:.o=.c)

clean:
	@rm -f $(BUILD)/*.elf
	@rm -f $(ISO)/*.elf
	@rm -f bin/*.iso

cleanObjects:
	@rm -f bin/build/objects/*.o

run:
	@qemu-system-x86_64 -cdrom $(ISO_FILE)