#ifndef C_GDT_IDT_LIB
#define C_GDT_IDT_LIB

#include "stdio.h"

/* Entry of Global Descriptor Table */
typedef struct GDT_Entry {
    // The lower 16 bits of the limit.
    uint16 limit_low;
    // The lower 16 bits of the base.
    uint16 base_low;
    // The next 8 bits of the base.
    uint8 base_middle;
    // Access flags, determine what ring this segment can be used in.
    uint8 access;
    uint8 granularity;
    // The last 8 bits of the base.
    uint8 base_high;
} __attribute__((packed)) GDT_Entry;

/* Pointer to Global Descriptor Table */
typedef struct GDT_Pointer {
    // The upper 16 bits of all selector limits.
    uint16 limit;
    // The address of the first GDT_Entry struct.
    uint32 base;
} __attribute__((packed)) GDT_Pointer;

/* Entry of Interrupt Descriptor Table */
typedef struct IDT_Entry {
    uint16 base_lo;             // The lower 16 bits of the address to jump to when this interrupt fires.
    uint16 sel;                 // Kernel segment selector.
    uint8  always0;             // This must always be zero.
    uint8  flags;               // More flags. See documentation.
    uint16 base_hi;             // The upper 16 bits of the address to jump to.
} __attribute__((packed)) IDT_Entry;

/* Pointer to Interrupt Descriptor Table */
typedef struct IDT_Pointer {
    uint16 limit;
    uint32 base;                // The address of the first element in our idt_entry_t array.
} __attribute__((packed)) IDT_Pointer;

// Load GDT/IDT
void loadDescriptorTables();

// ISR/IRQ that defined in Assembly
extern void isr0 ();
extern void isr1 ();
extern void isr2 ();
extern void isr3 ();
extern void isr4 ();
extern void isr5 ();
extern void isr6 ();
extern void isr7 ();
extern void isr8 ();
extern void isr9 ();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();

extern void irq0 ();
extern void irq1 ();
extern void irq2 ();
extern void irq3 ();
extern void irq4 ();
extern void irq5 ();
extern void irq6 ();
extern void irq7 ();
extern void irq8 ();
extern void irq9 ();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();

#endif