import copy
import json
from os import path, system


def parse_answer(text: str) -> bool:
    text = text.lower()
    return text == "y" or text == "yes"


class ConfigVariant:

    def __init__(self):
        self.props: dict[str, str] = {}
        self.flags: list[str] = []

class RunCommandBuilder:
    DEFAULT_QEMU_EXEC = "qemu-system-x86_64"

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
        output = RunCommandBuilder.DEFAULT_QEMU_EXEC

        if len(self.misc_props) > 0:
            output += ' '
            output += ' '.join('{} {}'.format(key, val) for key, val in self.misc_props.items())
            
        if len(self.misc_flags) > 0:
            output += ' '
            output += ' '.join(self.misc_flags)
        
        return output


def gen_variant(base_builder: RunCommandBuilder, name: str, variant: ConfigVariant) -> None:
    print(f"GENERATING VARIANT [{name}] ...")
    builder = copy.deepcopy(base_builder)
    builder.copy_from(variant)

    preprocess_variant(builder)

    filename = f"run_{name}.gen"
    filename += ".bat" if running_wsl else ".sh"

    filepath = path.join(currdir_path, filename)
    with open(filepath, "w") as file:
        if running_wsl:
            file.write(f"@set HERE_PATH={currdir_path}\n\n")

        file.write(builder.build())
        file.write("\n")
    
    if not running_wsl:
        system(f"chmod +x {filepath}")


def preprocess_variant(builder: RunCommandBuilder) -> None:
    if not running_wsl:
        return
    
    for preprocess_prop in configWslPathPreprocess:
        if preprocess_prop not in builder.misc_props:
            continue

        value = builder.misc_props[preprocess_prop]

        value = value.replace("/", "\\")
        value = "%HERE_PATH%\\" + value
        builder.misc_props[preprocess_prop] = value


def gen_all() -> None:
    base_builder = RunCommandBuilder()
    base_builder.copy_from(configBaseVariant)

    for varname, variant in configVariants.items():
        gen_variant(base_builder, varname, variant)


running_wsl = False
currdir_path: str = ""

configBaseVariant = ConfigVariant()
configVariants: dict[str, ConfigVariant] = {}
configWslPathPreprocess: list[str] = []


def config_read_varaint(config) -> ConfigVariant:
    variant = ConfigVariant()

    if "flags" in config:
        variant.flags = config["flags"]
    if "props" in config:
        variant.props = config["props"]

    return variant

def load_config() -> None:
    global configBaseVariant
    global configWslPathPreprocess

    with open("runscript-gen.json") as file:
        config = json.load(file)

    if "base" in config:
        configBaseVariant = config_read_varaint(config["base"])

    if "variants" in config:
        for varname, vardefinition in config["variants"].items():
            configVariants[varname] = config_read_varaint(vardefinition)

    if "wslPathPreprocess" in config:
        configWslPathPreprocess = config["wslPathPreprocess"]


if __name__ == "__main__":
    load_config()
    
    if len(configVariants) == 0:
        print("No variants defined for generation in the config.")
        exit(0)
    
    prompt = input("Are you running WSL? (y/n): ")
    running_wsl = parse_answer(prompt)

    if running_wsl:
        currdir_path = input("Ok. Please print the full network path to the project root: ")
        
        if not path.exists(currdir_path + "\\runscript-gen.py"):
            print("Sorry but seems like this is not the correct path. Exiting...")
            exit(-1)

        print("Seems like it's correct. Continuing...")

    print()
    gen_all()

