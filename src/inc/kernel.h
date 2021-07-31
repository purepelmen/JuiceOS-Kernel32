#ifndef C_KERNEL
#define C_KERNEL

#define KERNEL_VERSION "1.0.2-Beta4"

extern uint8_t* cpuid_get_id(void);
extern uint8_t* cpuid_get_model(void);

/* Initalize all descriptor tables, setup interrupts and so on... */
void kernel_init(void);
/* Kernel entry point */
void kernel_main(void);

/* Open console */
void console(void);
/* Open OS menu */
void openMenu(void);
/* Open OS information */
void openInfo(void);
/* Open Memory dumper */
void openMemoryDumper(void);
/* Open system logs */
void openSysLogs(void);
/* Print new string to logs */
void printLog(uint8_t* str);

#endif