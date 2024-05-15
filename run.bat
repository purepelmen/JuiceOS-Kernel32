@set THIS_FOLDER_PATH=\\wsl.localhost\Ubuntu-22.04\home\daniel\kernel32
@set ISO_PATH=%THIS_FOLDER_PATH%\build\artifacts\JuiceOS.iso

@echo Running QEMU...
qemu-system-x86_64 -hda %ISO_PATH% -m 256M -display sdl -vga std -name JuiceOS -machine q35
