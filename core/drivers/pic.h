#pragma once
#include <stdint.h>

#define PIC_MASTER_CMD  0x20
#define PIC_MASTER_DATA 0x21
#define PIC_SLAVE_CMD   0xA0
#define PIC_SLAVE_DATA  0xA1

void pic_init(uint8 masterOffset, uint8 slaveOffset);
void pic_disable();

void pic_mask(uint8 irq, bool isMasked);
uint8 pic_get_mask();

void pic_send_eoi(bool includingSlave);
