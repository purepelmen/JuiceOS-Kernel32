KERNEL_LINKER = bin/kernel.ld
ISO_PATH  = bin/JuiceOS.iso

OBJ_FILES = bin/build/objects
GRUB_FILES = bin/iso

KERNEL_ELF = $(GRUB_FILES)/juiceos_k32.elf
CPP_INCLUDES = include

CPP_FILES = $(shell find src -type f -name '*.cpp')
ASM_FILES = $(shell find src -type f -name '*.asm')
NEED_TO_COMPILE = $(patsubst src/%,$(OBJ_FILES)/%.o,$(CPP_FILES) $(ASM_FILES))

build: clean_and_init $(NEED_TO_COMPILE)
	ld -m elf_i386 -T $(KERNEL_LINKER) -o $(KERNEL_ELF) $(NEED_TO_COMPILE)
	
	@grub-mkrescue -V "JuiceOS" -o $(ISO_PATH) $(GRUB_FILES)
	@echo "Build successfull!"

clean_and_init:
	@rm -f -r $(OBJ_FILES)
	@mkdir -p $(OBJ_FILES)

clean:
	@rm -f $(ISO)/*.elf
	@rm -f bin/*.iso

run:
	@qemu-system-x86_64 -cdrom $(ISO_PATH)

$(OBJ_FILES)/%.cpp.o: src/%.cpp
	@mkdir -p $(dir $@)
	g++ -I $(CPP_INCLUDES) -nostdlib -ffreestanding -m32 -c $< -o $@

$(OBJ_FILES)/%.asm.o: src/%.asm
	@mkdir -p $(dir $@)
	nasm -f elf32 $< -o $@
