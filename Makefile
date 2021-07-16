build: init assembly
	@cat bin/build/boot.bin bin/build/kernel.bin > bin/OS.bin
	@echo "Build successfull!"

init:
	@mkdir -p bin/
	@mkdir -p bin/build/

assembly:
	@nasm -f bin -o bin/build/boot.bin src/boot.asm
	@nasm -f bin -o bin/build/kernel.bin src/kernel.asm

run:
	@qemu-system-x86_64 bin\os.bin