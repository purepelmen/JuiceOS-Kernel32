KERNEL_LINKER = bin/kernel.ld
ISO_PATH  = bin/JuiceOS.iso

OBJ_FILES = bin/build/objects
BUILD_DIRECTORY = bin/build
GRUB_FILES = bin/iso

CPP_INCLUDES = src/include

C_FILES = $(OBJ_FILES)/heap.o \
		  $(OBJ_FILES)/kernel.o \
		  $(OBJ_FILES)/stdlib.o \
		  $(OBJ_FILES)/string.o

C_DRIVERS_FILES = $(OBJ_FILES)/drivers/ps2.o \
		  		  $(OBJ_FILES)/drivers/screen.o \
		  		  $(OBJ_FILES)/drivers/ports.o

ASM_FILES = $(OBJ_FILES)/kernel_launcher.o

NEED_TO_COMPILE = $(ASM_FILES) $(C_FILES) $(C_DRIVERS_FILES)

build: cleanAndInitialize $(NEED_TO_COMPILE)
	ld -m elf_i386 -T $(KERNEL_LINKER) -o $(BUILD_DIRECTORY)/juiceos_k32.elf $(NEED_TO_COMPILE)
	
	@cp $(BUILD_DIRECTORY)/juiceos_k32.elf $(GRUB_FILES)/

	@grub-mkrescue -V "JuiceOS" -o $(ISO_PATH) $(GRUB_FILES)
	@echo "Build successfull!"

$(OBJ_FILES)/%.o: src/%.cpp
	g++ -I $(CPP_INCLUDES) -nostdlib -ffreestanding -m32 -c $< -o $@

$(OBJ_FILES)/drivers/%.o: src/drivers/%.cpp
	g++ -I $(CPP_INCLUDES) -nostdlib -ffreestanding -m32 -c $< -o $@

bin/build/objects/%.o: src/asm/%.asm
	nasm -f elf32 $< -o $@

cleanAndInitialize:
	@mkdir -p $(OBJ_FILES)/
	@mkdir -p $(OBJ_FILES)/drivers

	@rm -f $(OBJ_FILES)/*.o
	@rm -f $(OBJ_FILES)/drivers/*.o

clean:
	@rm -f $(BUILD_DIRECTORY)/*.elf
	@rm -f $(ISO)/*.elf
	@rm -f bin/*.iso

run:
	@qemu-system-x86_64 -cdrom $(ISO_PATH)
