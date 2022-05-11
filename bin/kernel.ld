ENTRY(start)

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

    end = .; _end = .; __end = .;
}