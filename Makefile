ISO_FILE  = bin/JuiceOS.iso
LINK 	  = bin/linkers
BUILD 	  = bin/build
OBJECTS	  = bin/build/objects
ISO 	  = bin/iso

C_FILES = $(OBJECTS)/heap.o \
		  $(OBJECTS)/kernel.o \
		  $(OBJECTS)/stdlib.o

C_DRIVERS_FILES = $(OBJECTS)/drivers/ps2.o \
		  		  $(OBJECTS)/drivers/screen.o \
		  		  $(OBJECTS)/drivers/ports.o

ASM_FILES = $(OBJECTS)/kernel_launcher.o

NEED_TO_COMPILE = $(ASM_FILES) $(C_FILES) $(C_DRIVERS_FILES)

<<<<<<< HEAD
build: cleanAndInitialize $(NEED_TO_COMPILE)
=======
NEED_TO_COMPILE = $(ASM_FILES) $(C_FILES) $(C_DRIVERS_FILES) $(C_ISR_FILES)

installdep:
	sudo apt install nasm gcc grub2 build-essential mtools
	

build: delObjsAndInitFolders $(NEED_TO_COMPILE)
>>>>>>> 0a3758836cbb95572ff3aeb245abf72cd0d762b6
	ld -m elf_i386 -T $(LINK)/kernel.ld -o $(BUILD)/juiceos_k32.elf $(NEED_TO_COMPILE)
	
	@cp $(BUILD)/juiceos_k32.elf $(ISO)/

	@grub-mkrescue -V "JuiceOS" -o $(ISO_FILE) $(ISO)
	@echo "Build successfull!"

$(OBJECTS)/%.o: src/%.c
	gcc -ffreestanding -m32 -c $< -o $@

$(OBJECTS)/drivers/%.o: src/drivers/%.c
	gcc -ffreestanding -m32 -c $< -o $@

bin/build/objects/%.o: src/asm/%.asm
	nasm -f elf32 $< -o $@

cleanAndInitialize:
	@mkdir -p $(OBJECTS)/
	@mkdir -p $(OBJECTS)/drivers
	@mkdir -p $(OBJECTS)/isr

	@rm -f $(OBJECTS)/*.o
	@rm -f $(OBJECTS)/drivers/*.o
	@rm -f $(OBJECTS)/isr/*.o

clean:
	@rm -f $(BUILD)/*.elf
	@rm -f $(ISO)/*.elf
	@rm -f bin/*.iso

run:
	@qemu-system-x86_64 -cdrom $(ISO_FILE)
