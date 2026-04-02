import copy
import json
from os import system
import os
from shutil import which
import sys


def parse_answer(text: str) -> bool:
    text = text.lower()
    return text == "y" or text == "yes"


class ConfigVariant:

    def __init__(self):
        self.props: dict[str, str] = {}
        self.flags: list[str] = []

class RunCommandBuilder:
    DEFAULT_QEMU_EXEC = "qemu-system-i386"

    def __init__(self):
        self.misc_props: dict[str, str] = {}
        self.misc_flags: list[str] = []

    def copy_from(self, variant: ConfigVariant) -> None:
        for key, value in variant.props.items():
            self.misc_props[key] = value

        for flag in variant.flags:
            if flag in self.misc_flags:
                continue

            self.misc_flags.append(flag)
    
    def build(self) -> str:
        output = f"emul=\"{RunCommandBuilder.DEFAULT_QEMU_EXEC}\"\n"

        output += "args=\""
        if len(self.misc_props) > 0:
            output += ' '
            output += ' '.join('{} {}'.format(key, val) for key, val in self.misc_props.items())
            
        if len(self.misc_flags) > 0:
            output += ' '
            output += ' '.join(self.misc_flags)
        
        output += "\"\n\n"
        
        return output


def gen_variant(base_builder: RunCommandBuilder, name: str, variant: ConfigVariant) -> None:
    print(f"GENERATING VARIANT [{name}] ...")
    builder = copy.deepcopy(base_builder)
    builder.copy_from(variant)

    preprocess_variant(builder)

    filename = f"run_{name}.gen.sh"
    filepath = os.path.join(currdir_path, filename)
    with open(filepath, "w") as file:
        file.write(builder.build())
        if running_wsl:
            file.write("powershell.exe -Command Start-Process -FilePath $emul -ArgumentList \\\"$args\\\"")
        else:
            file.write("$emul $args")
        
        file.write("\n")
    
    system(f"chmod +x {filepath}")


def preprocess_variant(builder: RunCommandBuilder) -> None:
    if not running_wsl:
        return
    
    for preprocess_prop in config_wsl_path_preprocess:
        if preprocess_prop not in builder.misc_props:
            continue

        value = builder.misc_props[preprocess_prop]
        value = f"$(wslpath -aw {value})"
        builder.misc_props[preprocess_prop] = value


def gen_all() -> None:
    base_builder = RunCommandBuilder()
    base_builder.copy_from(config_base)

    if make_debug_variants:
        debug_builder = RunCommandBuilder()
        debug_builder.copy_from(config_base)
        debug_builder.copy_from(config_base_debug)

    for varname, variant in config_variants.items():
        gen_variant(base_builder, varname, variant)

        if make_debug_variants:
            gen_variant(debug_builder, f"{varname}-debug", variant)


running_wsl = False
currdir_path: str = ""
make_debug_variants = False

config_base = ConfigVariant()
config_base_debug = ConfigVariant()
config_variants: dict[str, ConfigVariant] = {}
config_wsl_path_preprocess: list[str] = []


def config_read_varaint(config) -> ConfigVariant:
    variant = ConfigVariant()

    if "flags" in config:
        variant.flags = config["flags"]
    if "props" in config:
        variant.props = config["props"]

    return variant

def load_config() -> None:
    global config_base
    global config_base_debug
    global config_wsl_path_preprocess

    with open("runscript-gen.json") as file:
        config = json.load(file)

    if "base" in config:
        config_base = config_read_varaint(config["base"])
    if "debug" in config:
        config_base_debug = config_read_varaint(config["debug"])

    if "variants" in config:
        for varname, vardefinition in config["variants"].items():
            config_variants[varname] = config_read_varaint(vardefinition)

    if "wslPathPreprocess" in config:
        config_wsl_path_preprocess = config["wslPathPreprocess"]


if __name__ == "__main__":
    if "-h" in sys.argv or "--help" in sys.argv or "--usage" in sys.argv:
        print(f"Usage: {sys.argv[0]} [--no-wsl] [--debug]")
        exit(0)

    load_config()
    if len(config_variants) == 0:
        print("No variants defined for generation in the config.")
        exit(0)

    wslpath_path = which("wslpath")
    running_wsl = wslpath_path is not None

    if "--no-wsl" in sys.argv:
        running_wsl = False

    if running_wsl:
        print("WSL detected. The scripts will be generated for it and Qemu in Windows.")
        print("Use --no-wsl option to force generation of normal scripts.")
    else:
        print("Normal Linux scripts generation started (non-WSL).")

    if "--debug" in sys.argv:
        make_debug_variants = True
        print("\n> Enabled Debug subvariants generation.")

    print()
    gen_all()

