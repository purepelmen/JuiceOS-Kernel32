# JuiceOS-Kernel32

## Requirements
Type this to the console:
```console
sudo apt install build-essential
```
```console
sudo apt install nasm
```
```console
sudo apt install grub2
```
```console
sudo apt install xorriso
```
Windows users should install **Qemu for Windows**. Linux users should install this:
```console
sudo apt install qemu-system-x86
```
## Running in emulator
Linux:
```console
make run
```
Windows:

Change path to the ISO file in `run.bat` and run it
## Running on a real hardware
For linux users:
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
