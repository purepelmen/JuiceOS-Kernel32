**! Development was stopped indefinitely. Not all parts of this kernel can work as expected even on emulators. !**

# JuiceOS-Kernel32
JuiceOS Kernel32 - my x86 hobby OS kernel.

## Requirements
This project is supposed to be built on Linux only. You may use WSL as well.

```console
sudo apt install build-essential cmake ninja-build nasm
```
```console
sudo apt install grub2 xorriso
```


## Running in an emulator
### Requirements
Windows users should install **Qemu for Windows**, and add Qemu to Windows environment variables.

Linux users should install this:
```console
sudo apt install qemu-system-x86
```

### Testing
Linux:
```console
make run
```

Windows:
Make the path in `run.bat` be correct for your computer, so it'll point to the built ISO file. Then run it.

## Running on a real hardware
For Linux users:
```console
sudo dd -if bin/JuiceOS.iso -of /dev/sdb
```
where `/dev/sdb` - is your USB drive.

For Windows users:

You need to have Rufus. So after installing Rufus, open it, and drag and drop `bin/JuiceOS.iso` file to the Rufus window. After this your window must be like this:

![alt text](https://github.com/purepelmen/JuiceOS-Kernel32/blob/master/docs/booting-on-real-hardware/flashing_settings_preview.png?raw=true)

Then click **START** button. In this popup select the second item:

![alt text](https://github.com/purepelmen/JuiceOS-Kernel32/blob/master/docs/booting-on-real-hardware/image_write_mode.png?raw=true)

And then confirm flashing:

![alt text](https://github.com/purepelmen/JuiceOS-Kernel32/blob/master/docs/booting-on-real-hardware/flashing_confirm.png?raw=true)

That's all. But remember, **THIS OS WILL WORK ONLY ON PC WITH LEGACY BIOS or UEFI-CSM!**
