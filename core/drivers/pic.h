#pragma once
#include <stdint.h>

#define PIC_MASTER_CMD  0x20
#define PIC_MASTER_DATA 0x21
#define PIC_SLAVE_CMD   0xA0
#define PIC_SLAVE_DATA  0xA1

void pic_init(uint8 masterOffset, uint8 slaveOffset);
void pic_disable();

void pic_mask(uint8 irq, bool isMasked);
uint16 pic_get_mask();

/// @brief Get In-Service Register combined from both master and slave PICs.
uint16 pic_get_isr();

void pic_send_eoi(bool includingSlave);
