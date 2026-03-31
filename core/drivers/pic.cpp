#include "pic.h"
#include "ports.h"

#define PIC_EOI 0x20

void pic_init(uint8 masterOffset, uint8 slaveOffset)
{
    port_write8(PIC_MASTER_CMD, 0x11);
    port_write8(PIC_SLAVE_CMD, 0x11);

    port_write8(PIC_MASTER_DATA, masterOffset);
    port_write8(PIC_SLAVE_DATA, slaveOffset);

    port_write8(PIC_MASTER_DATA, 0x04);
    port_write8(PIC_SLAVE_DATA, 0x02);

    port_write8(PIC_MASTER_DATA, 0x01);
    port_write8(PIC_SLAVE_DATA, 0x01);

    // Unmasking both PICs.
    port_write8(PIC_MASTER_DATA, 0x0);
    port_write8(PIC_SLAVE_DATA, 0x0);
}

void pic_disable()
{
    // Masks all interrupts on both PICs.
    port_write8(PIC_MASTER_DATA, 0xFF);
    port_write8(PIC_SLAVE_DATA, 0xFF);
}

void pic_mask(uint8 irq, bool isMasked)
{
    uint16 port = irq < 8 ? PIC_MASTER_DATA : PIC_SLAVE_DATA;
    if (irq >= 8)
        irq -= 8;

    uint8 newMask = port_read8(port);
    if (isMasked)
        newMask = newMask | (1 << irq);
    else
        newMask = newMask & ~(1 << irq);

    port_write8(port, newMask);
}

uint16 pic_get_mask()
{
    return port_read8(PIC_MASTER_DATA) | (port_read8(PIC_SLAVE_DATA) << 8);
}

uint16 pic_get_isr()
{
    port_write8(PIC_MASTER_CMD, 0x0B);
    port_write8(PIC_SLAVE_CMD, 0x0B);

    return (port_read8(PIC_SLAVE_CMD) << 8) | port_read8(PIC_MASTER_CMD);
}

void pic_send_eoi(bool includingSlave)
{
    if (includingSlave)
        port_write8(PIC_SLAVE_CMD, PIC_EOI);
    
    port_write8(PIC_MASTER_CMD, PIC_EOI);
}
