#ifndef C_KERNEL
#define C_KERNEL

#define KERNEL_VERSION "1.0.2-Beta3"

extern uint8_t* cpuid_get_id(void);
extern uint8_t* cpuid_get_model(void);

void kernel_main(void);

void console(void);
void openMenu(void);
void openInfo(void);
void openMemoryDumper(void);

#endif