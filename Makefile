ISO_FILE  = bin/JuiceOS.iso
LINK 	  = bin/linkers
BUILD 	  = bin/build
OBJECTS	  = bin/build/objects
ISO 	  = bin/iso

CPP_INCLUDES = src/include

C_FILES = $(OBJECTS)/heap.o \
		  $(OBJECTS)/kernel.o \
		  $(OBJECTS)/stdlib.o \
		  $(OBJECTS)/string.o

C_DRIVERS_FILES = $(OBJECTS)/drivers/ps2.o \
		  		  $(OBJECTS)/drivers/screen.o \
		  		  $(OBJECTS)/drivers/ports.o

ASM_FILES = $(OBJECTS)/kernel_launcher.o

NEED_TO_COMPILE = $(ASM_FILES) $(C_FILES) $(C_DRIVERS_FILES)

build: cleanAndInitialize $(NEED_TO_COMPILE)
	ld -m elf_i386 -T $(LINK)/kernel.ld -o $(BUILD)/juiceos_k32.elf $(NEED_TO_COMPILE)
	
	@cp $(BUILD)/juiceos_k32.elf $(ISO)/

	@grub-mkrescue -V "JuiceOS" -o $(ISO_FILE) $(ISO)
	@echo "Build successfull!"

$(OBJECTS)/%.o: src/%.cpp
	g++ -I $(CPP_INCLUDES) -nostdlib -ffreestanding -m32 -c $< -o $@

$(OBJECTS)/drivers/%.o: src/drivers/%.cpp
	g++ -I $(CPP_INCLUDES) -nostdlib -ffreestanding -m32 -c $< -o $@

bin/build/objects/%.o: src/asm/%.asm
	nasm -f elf32 $< -o $@

cleanAndInitialize:
	@mkdir -p $(OBJECTS)/
	@mkdir -p $(OBJECTS)/drivers

	@rm -f $(OBJECTS)/*.o
	@rm -f $(OBJECTS)/drivers/*.o

clean:
	@rm -f $(BUILD)/*.elf
	@rm -f $(ISO)/*.elf
	@rm -f bin/*.iso

run:
	@qemu-system-x86_64 -cdrom $(ISO_FILE)
