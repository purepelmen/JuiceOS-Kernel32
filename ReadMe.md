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

## My development environment
I use VSCode. If I use WSL, it's of course installed inside Windows. I also use Qemu, and the instructions will be only for this emulator. The important point is that I don't install Qemu inside WSL. As stated above "you should install Qemu for Windows, and add it to env variables". 

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
After you've generated the run scripts via `runscript-gen.py` you should edit them (for all configurations) and add `-s -S`. The former flag is a shortcut for `-gdb tcp::1234`, and the latter freezes CPU at startup and waits until debugger is connected. 

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
            "command": "powershell.exe",
            "args": [
                "-Command",
                "Start-Process -FilePath './run_ide.gen.bat'"
            ],
            "dependsOn": "Build kernel"
        }
    ]
}
```

First task compiles the kernel and assembles the ISO using `make`, the second one depends on the first and runs Qemu through the generated run script (I use PowerShell here because CMD.EXE doesn't support UNC paths). On pure Linux, edit this configuration so you run the shell script directly. 

Also make sure to select the desired run preset (`run_ide`, `run_default`...).

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
