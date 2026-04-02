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
Windows users can install **Qemu for Windows**, and add Qemu to Windows environment variables. The other way is to install Qemu inside the WSL as Linux users should do this.

Linux users should install this:
```console
sudo apt install qemu-system-x86
```

Some presets for running Qemu are defined in `runscript-gen.json`, and you can use `runscript-gen.py` to generate them for both Linux and WSL. For this you'll need Python 3:

```console
sudo apt install python3
```

### Testing

#### Generate run scripts
Run this in your Linux terminal or inside the WSL shell if you use it:
```console
make gen-runs
```
or directly run the script:
```console
python3 runscript-gen.py
```

If you're inside a WSL distribution, the script will know it and generate `.sh` scripts accordingly to use **Qemu for Windows**. If this is undesirable and you want to use Qemu installed inside the WSL distro, add a special flag to the above command so you get: `python3 runscript-gen.py --no-wsl`.

After that run scripts will be generated. Re-run the script if you make changes to the JSON config.

#### Run the emulator
Redardless if you use WSL or not, the scripts are `.sh` (Bash scripts). It's not anymore Batch/PowerShell as it was before.

When using WSL you must run them from the WSL terminal (inside VSCode f.i.). You can specify the script inside VSCode tasks (in WSL this will only work if VSCode is connected to your WSL distro).

If you don't know which run script to better use, currently I recommend the `ide` variant as the OS disk drivers don't work on the default variant yet (where there's AHCI controller).

## My development environment
I use VSCode. If I use WSL, it's of course installed inside Windows. I also use Qemu, and the instructions will be only for this emulator. I don't install Qemu inside WSL, though you can.

If I use WSL I of course open the WSL folder inside VSCode by connecting to it, because right-clicking in the Windows explorer and selecting "Open in VSCode" is not enough. For that you must have `ms-vscode-remote.remote-wsl` extension which should be suggested automatically (probably).

Next I use these vscode extensions for language support: `ms-vscode.cpptools`, `doinkythederp.nasm-language-support`.

It's also convenient to have these (but it's optional): `ms-vscode.hexeditor`, `mhutchie.git-graph`.

**Note though** if you have `ms-vscode.cmake-tools` installed, when you open the folder with this project, it detects CMakeLists.txt and asks to configure it. Click ESCAPE! It tries to automatically configure and build everything. We don't need that. The Makefile in the root directory must handle the CMake project by itself.

### Configuring extensions
Your `make` command may build everything correctly, but IntelliSense will show errors somewhere if you don't set up everything properly. Create and edit `.vscode/settings.json`:
```json
{
    "nasm.outputFormat": "elf32",
    "C_Cpp.default.intelliSenseMode": "gcc-x86",
    "C_Cpp.default.includePath": [ 
        "core/**"
    ],
    "C_Cpp.default.systemIncludePath": []
}
```

The NASM extension should know that we use 32-bit format (very important). C/C++ IntelliSense also must know we're compilling for x86 architecture. We also need to set `systemIncludePath` empty, so the standard library of the system won't conflict with some files in this project like `stdint.h`, `stdlib.h` and so on.

### Making everything Build&Run at F5 hotkey (+ Kernel debugging)
For debugging to work some special flags must be passed to the emulator. When you generate the run scripts via `runscript-gen.py` you should add an additional flag "debug" which tells to generate a debug version of every variant:

```console
python3 runscript-gen.py --debug
```

Added flags make sure the debug server will run at `tcp::1234`, freeze CPU at startup, and wait until debugger is connected. So you run debug run scripts only when you need to debug.

Then create and edit `.vscode/tasks.json`:
```json
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "Build kernel",
            "type": "shell",
            "command": "make",
            "group": "build",
            "problemMatcher": [ "$gcc" ]
        },
        {
            "label": "Run QEMU",
            "type": "shell",
            "command": "./run_ide-debug.gen.sh",
            "args": [],
            "dependsOn": "Build kernel"
        }
    ]
}
```

First task compiles the kernel and assembles the ISO using `make`, the second one depends on the first and runs the generated run script. Make sure to select the desired run preset (`run_ide`, `run_default`...).

This is not enough. The last step would be to add a launch configuration (that you will actually launch when pressing F5). Create and edit `.vscode/launch.json`:
```json
{
    // Use IntelliSense to learn about possible attributes.
    // Hover to view descriptions of existing attributes.
    // For more information, visit: https://go.microsoft.com/fwlink/?linkid=830387
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Kernel Debug",
            "type": "cppdbg",
            "request": "launch",
            "preLaunchTask": "Run QEMU",

            "program": "${workspaceFolder}/build/iso/juiceos_k32.elf",
            "cwd": "${workspaceFolder}",

            "externalConsole": false,
            "stopAtEntry": true,

            "MIMode": "gdb",
            "miDebuggerServerAddress": "172.27.16.1:1234",
            "targetArchitecture": "x86",
            "setupCommands": [
                {
                    "description": "Enable pretty-printing for gdb",
                    "text": "-enable-pretty-printing",
                    "ignoreFailures": true
                }
            ]
        }

    ]
}
```

**Very important thing** in the `miDebuggerServerAddress`. It's the actual address of the running Qemu GDB server. If you're on pure Linux, put there `127.0.0.1:1234` (the server will always be at localhost and on the port 1234 according to the passed Qemu debug arguments). 

If you're using WSL, you need to access Qemu that is launched outside the WSL. Your WSL networking mode is probably NAT (otherwise make sure to set it). In this case you should enter `ip route show` in the WSL terminal, and at the `default via...` route you'll see your Windows IP-address which you can use to access Qemu. Another option would be to run `cat /etc/resolv.conf` and see the nameserver field where you can find the IP address. Then put it in the `miDebuggerServerAddress` property. I heard this IP address may change (didn't encounter this yet), so if something stops working later, replace the address again. The port should be 1234 as well.

That's it. Now click F5 and... everything just works (though on WSL you probably need to confirm the security window every time). Re-running after making some small code changes should be pretty fast. Also you can stop the execution, place some breakpoints, inspect the registers.

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
