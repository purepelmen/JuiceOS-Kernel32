ISO_FILE  = bin/JuiceOS.iso
LINK 	  = bin/linkers
BUILD 	  = bin/build
OBJECTS	  = bin/build/objects
ISO 	  = bin/iso

C_FILES = $(OBJECTS)/gdt_idt.o \
		  $(OBJECTS)/heap.o \
		  $(OBJECTS)/isr.o \
		  $(OBJECTS)/kernel.o \
		  $(OBJECTS)/ports.o \
		  $(OBJECTS)/ps2.o \
		  $(OBJECTS)/screen.o \
		  $(OBJECTS)/stdio.o \
		  $(OBJECTS)/system.o

ASM_FILES = $(OBJECTS)/interrupts.o \
			$(OBJECTS)/kernel_launcher.o

build: cleanObjects $(ASM_FILES) $(C_FILES)
	ld -m elf_i386 -T $(LINK)/kernel.ld -o $(BUILD)/juiceos_k32.elf $(ASM_FILES) $(C_FILES)
	
	@cp $(BUILD)/juiceos_k32.elf $(ISO)/

	@grub-mkrescue -V "JuiceOS" -o $(ISO_FILE) $(ISO)
	@echo "Build successfull!"

bin/build/objects/%.o: src/%.c
	gcc -ffreestanding -m32 -c $< -o $@

bin/build/objects/%.o: src/asm/%.asm
	nasm -f elf32 $< -o $@

cleanObjects:
	@rm -f bin/build/objects/*.o

clean:
	@rm -f $(BUILD)/*.elf
	@rm -f $(ISO)/*.elf
	@rm -f bin/*.iso

run:
	@qemu-system-x86_64 -cdrom $(ISO_FILE)
