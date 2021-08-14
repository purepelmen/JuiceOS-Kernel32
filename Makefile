ISO_FILE  = bin/JuiceOS.iso
LINK 	  = bin/linkers
BUILD 	  = bin/build
OBJECTS	  = bin/build/objects
ISO 	  = bin/iso

C_FILES = $(OBJECTS)/gdt_idt.o \
		  $(OBJECTS)/heap.o \
		  $(OBJECTS)/isr.o \
		  $(OBJECTS)/kernel.o \
		  $(OBJECTS)/stdio.o \
		  $(OBJECTS)/system.o

C_DRIVERS_FILES = $(OBJECTS)/drivers/ps2.o \
		  		  $(OBJECTS)/drivers/screen.o \
		  		  $(OBJECTS)/drivers/ports.o

ASM_FILES = $(OBJECTS)/interrupts.o \
			$(OBJECTS)/kernel_launcher.o

build: cleanObjects $(ASM_FILES) $(C_FILES) $(C_DRIVERS_FILES)
	ld -m elf_i386 -T $(LINK)/kernel.ld -o $(BUILD)/juiceos_k32.elf $(ASM_FILES) $(C_FILES) $(C_DRIVERS_FILES)
	
	@cp $(BUILD)/juiceos_k32.elf $(ISO)/

	@grub-mkrescue -V "JuiceOS" -o $(ISO_FILE) $(ISO)
	@echo "Build successfull!"

$(OBJECTS)/%.o: src/%.c
	gcc -ffreestanding -m32 -c $< -o $@

$(OBJECTS)/drivers/%.o: src/drivers/%.c
	gcc -ffreestanding -m32 -c $< -o $@

bin/build/objects/%.o: src/asm/%.asm
	nasm -f elf32 $< -o $@

cleanObjects:
	@rm -f $(OBJECTS)/*.o
	@rm -f $(OBJECTS)/drivers/*.o

clean:
	@rm -f $(BUILD)/*.elf
	@rm -f $(ISO)/*.elf
	@rm -f bin/*.iso

run:
	@qemu-system-x86_64 -cdrom $(ISO_FILE)
