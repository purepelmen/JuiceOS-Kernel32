ARTIFACTS = build/artifacts
K32CORE_ARTIFACTS = $(ARTIFACTS)/k32-core/
BUILT_ISO  = $(ARTIFACTS)/JuiceOS.iso

K32CORE_LINKERSCRIPT = build/kernel.ld
ISOBUILD_DIR = build/iso
K32CORE_LINKED_EXECUTABLE = $(ISOBUILD_DIR)/juiceos_k32.elf

FS_DIR = build/fs
MAIN_PARTITION_FILE = $(FS_DIR)/partition.img

gen-iso: build-core
	@grub-mkrescue -V "JuiceOS" -o $(BUILT_ISO) $(ISOBUILD_DIR) -append_partition 2 0x00 $(MAIN_PARTITION_FILE)
	@echo ".ISO Generated!"

build-core:
	@echo "Configuring Kernel32-Core CMake project and building..."
	@cmake -S core -B $(K32CORE_ARTIFACTS) -G Ninja
	@ninja -C $(K32CORE_ARTIFACTS)
	
	@echo "Linking the output K32CORE executable..."
	@ld -m elf_i386 -T $(K32CORE_LINKERSCRIPT) -o $(K32CORE_LINKED_EXECUTABLE) $(K32CORE_ARTIFACTS)/libK32_Core.a 

	@echo "Build successfull!"

clean:
	@rm -rf $(ARTIFACTS)
	@rm -f $(K32CORE_LINKED_EXECUTABLE)

clean-generated:
	@rm -f *.gen.sh
	@rm -f *.gen.bat

gen-runs:
	@python3 runscript-gen.py

run:
	@qemu-system-x86_64 -cdrom $(BUILT_ISO)
