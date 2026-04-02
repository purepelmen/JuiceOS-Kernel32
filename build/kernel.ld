ENTRY(start)
OUTPUT_FORMAT(elf32-i386)

SECTIONS {
    .boot 0x100000:
    {
        *(.multiboot_header)
    }

    .text :
    {
        *(.text)
    }

    .rodata :
    {
        *(.rodata)
    }

    .data :
    {
        *(.data)
        *(.rodata)
    }

    .bss :
    {
        *(.bss)
    }

    .init_array :
    {
        __init_array_start = .;
        KEEP (*(SORT(.init_array.*)))
        KEEP (*(.init_array))
        __init_array_end = .;
    }

    end = .; _end = .; __end = .;
}