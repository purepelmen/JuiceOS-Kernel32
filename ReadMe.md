# JuiceOS-Kernel32
JuiceOS Kernel32 - my x86 hobby OS kernel.

## Build Requirements
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

Some presets for running Qemu are defined in `runscript-gen.json`, and you can use `runscript-gen.py` to generate them for both Linux and WSL. For this you'll need Python 3. Install it on Windows (not in WSL) if you're using WSL. 

If you're using Linux without WSL, just install Python 3:
```console
sudo apt install python3
```

### Testing

#### Generate run scripts
For normal Linux (not WSL) either run this:
```console
make gen-runs
```
or directly run the script:
```console
python3 runscript-gen.py
```

When using WSL, run the `runscript-gen.py` script in the Windows environment.

Then follow the instructions, and the run scripts will be generated. Re-run the script if you make changes to the JSON config.

#### Run the emulator
Use any of the generated run scripts. When using WSL you can just double-click a .bat script in the explorer. On Linux run a .sh script. They all are already marked executable.

If you don't know which run script to better use, run the default/normal variant. 


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
