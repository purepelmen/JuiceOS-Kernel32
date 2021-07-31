#ifndef C_KERNEL
#define C_KERNEL

#define KERNEL_VERSION "1.0.2-Beta4"

extern uint8_t* cpuid_get_id(void);
extern uint8_t* cpuid_get_model(void);

void kernel_main(void);

void console(void);
void openMenu(void);
void openInfo(void);
void openMemoryDumper(void);
void openSysLogs(void);
void printLog(uint8_t* str);

#endif