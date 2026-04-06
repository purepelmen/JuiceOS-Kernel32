#include "ports.h"
#include "pit.h"
#include "../kernel.h"
#include "../isr.h"

#define PIT_FREQ_HZ 1193181.666

enum class pit_rwmode : uint8
{
    LATCH_COUNTER = 0b00,
    ONLY_LOWBYTE = 0b01,
    ONLY_HIGHBYTE = 0b10,
    LOW_AND_HIGH_BYTE = 0b11,
};

enum class pit_channelmode : uint8
{
    MODE_0 = 0b000,  // Interrupt on Terminal Count.
    MODE_1 = 0b001,
    MODE_2 = 0b010,  // Rate Generator.
    MODE_3 = 0b011,  // Square Wave Generator.
    MODE_4 = 0b100,  // Software Triggered Strobe.
    MODE_5 = 0b101
};

struct pit_command
{
    bool useBcdNumbers : 1;
    pit_channelmode mode : 3;
    pit_rwmode rwMode : 2;
    uint8 channel : 2;

    uint8 as_byte() const { return *((uint8*)this); }
} __attribute__((packed));

namespace ktimer
{
    // We get 100 HZ, or 10 ms per tick.
    const int DIVISOR = (PIT_FREQ_HZ / 100.0);
    const int DEFAULT_FREQ = (float)PIT_FREQ_HZ / DIVISOR;
    const int MS_PER_TICK = 1000.0 / DEFAULT_FREQ;

    static volatile unsigned long long ticks = 0ULL;

    static void timer_handler(const kisr::regs_t& regs);
    
    void init()
    {
        pit_command cmd = {0};
        cmd.rwMode = pit_rwmode::LOW_AND_HIGH_BYTE;
        cmd.mode = pit_channelmode::MODE_2;

        port_write8(0x43, cmd.as_byte());
        port_write8(0x40, DIVISOR & 0xFF);
        port_write8(0x40, (DIVISOR >> 8) & 0xFF);

        kisr::register_handler(IRQ_BASE, timer_handler);
        kernel_log("Timer was initialized.\n");
    }

    void wait(uint32 ms)
    {
        static_assert(MS_PER_TICK > 0);

        auto goal = ticks + ms / MS_PER_TICK;
        while(ticks < goal)
        {
            asm volatile("hlt");
        }
    }

    void timer_handler(const kisr::regs_t& regs)
    {
        ticks++;
    }
}
