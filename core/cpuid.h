#pragma once
#include "stdint.h"

struct cpuid_q1eax
{
    uint8 modification : 4;
    uint8 model : 4;
    uint8 family : 4;
    uint8 type : 2;
    uint32 __reserved : 18;
} __attribute__((packed));

/// @brief CPU extensions provided by CPUID EAX=01h (EBX extensions). May be garbage on some old CPUs.
struct cpuid_q1ebx
{
    bool BRAND_INDEX : 1;
    uint8 __reserved : 7;
    bool CLFLUSH_LINE_SIZE : 1;
    uint8 __reserved2 : 7;
    bool APIC_ID_SPACE : 1;
    uint8 __reserved3 : 7;
    bool INITIAL_APIC_ID : 1;
    uint8 __reserved4 : 7;

} __attribute__((packed));

/// @brief CPU extensions provided by CPUID EAX=01h (ECX extensions). May be garbage on some old CPUs.
struct cpuid_q1ecx
{
    bool SSE3 : 1;
    bool PCLMUL : 1;
    bool DTES64 : 1;
    bool MONITOR : 1;
    bool DS_CPL : 1;
    bool VMX : 1;
    bool SMX : 1;
    bool EST : 1;
    bool TM2 : 1;
    bool SSSE3 : 1;
    bool CID : 1;
    bool SDBG : 1;
    bool FMA : 1;
    bool CX16 : 1;
    bool XTPR : 1;
    bool PDCM : 1;
    bool PCID : 1;
    bool DCA : 1;
    bool SSE4_1 : 1;
    bool SSE4_2 : 1;
    bool X2APIC : 1;
    bool MOVBE : 1;
    bool POPCNT : 1;
    bool TSC : 1;
    bool AES : 1;
    bool XSAVE : 1;
    bool OSXSAVE : 1;
    bool AVX : 1;
    bool F16C : 1;
    bool RDRAND : 1;
    bool HYPERVISOR : 1;

} __attribute__((packed));

/// @brief CPU extensions provided by CPUID EAX=01h.
struct cpuid_q1edx
{
    bool FPU : 1;
    bool VME : 1; // Probably V86 mode.
    bool DE : 1;
    bool PSE : 1; // 4MB pages.
    bool TSC : 1; // Support for `rdtsc` and `CR4.TSC`.
    bool MSR : 1;
    bool PAE_or_PTE : 1; // PAE, or PTE (Cyrix only).
    bool MCE : 1;
    bool CX8 : 1; // Support for `cmpxchg8b`.
    bool APIC_or_PGE : 1; // APIC, or PGE (AMD only).
    bool __reserved : 1;
    bool SEP : 1; // Support for `sysenter`/`sysexit` (Pentium II).
    bool MTRR : 1; // Support for MTRR model-specific registers.
    bool PGE : 1; // Support for `CR4.PGE`, global paging flags.
    bool MCA : 1;
    bool CMOV : 1;
    bool PAT : 1;

    bool PSE36 : 1;
    bool PSN : 1;
    bool CFLUSH : 1;
    uint8 __reserved2 : 1;
    bool DS : 1;
    bool ACPI : 1;

    bool MMX : 1;
    bool FXSR : 1; // Support for MMX2.
    bool SSE : 1; // SSE extensions (Pentium III).
    bool SSE2 : 1; // SSE2 extensions.
    bool SS : 1;
    bool HTT : 1;
    bool TM : 1;
    bool IA64 : 1;
    bool PBE : 1;
} __attribute__((packed));

struct cpuid_q1
{
    cpuid_q1eax versionInfo;
    cpuid_q1ebx ebxExt;
    cpuid_q1ecx ecxExt;
    cpuid_q1edx ext;
} __attribute__((packed));


static_assert(sizeof(cpuid_q1eax) == 4);
static_assert(sizeof(cpuid_q1ebx) == 4);
static_assert(sizeof(cpuid_q1ecx) == 4);
static_assert(sizeof(cpuid_q1edx) == 4);


extern "C" bool is_cpuid_supported();

/// @brief Queries basic CPUID info with EAX=0h.
/// @param outVendorId An output string buffer for Vendor ID sized 12 + 1 (null term) bytes.
/// @return Maximum number of available query level for EAX.
int cpuid(char* outVendorId)
{
    int result;
    
    int vendorId[3];
    __asm__("cpuid" : "=a"(result), "=b"(vendorId[0]), "=c"(vendorId[2]), "=d"(vendorId[1]) : "a"(0));

    int* out = (int*)outVendorId;
    for (size_t i = 0; i < 3; i++)
    {
        out[i] = vendorId[i];
    }
    
    outVendorId[12] = 0x0;
    return result;
}

cpuid_q1 cpuid_eax1()
{
    int cpuVersion, extensions, ebxExtensions, ecxExtensions;
    __asm__("cpuid" : "=a"(cpuVersion), "=b"(ebxExtensions), "=c"(ecxExtensions), "=d"(extensions) : "a"(1));

    cpuid_q1 result;
    result.ebxExt = *(cpuid_q1ebx*)(&extensions);
    result.ecxExt = *(cpuid_q1ecx*)(&extensions);
    result.ext = *(cpuid_q1edx*)(&extensions);
    result.versionInfo = *(cpuid_q1eax*)(&cpuVersion);

    return result;
}

/// @brief Performs CPUID with EAX=80000002h.
/// @param cpuinfo_buffer Must be at least 48-byte + 1 (null term) sized. 
void cpuid_80000002h(char* outCpuInfoBuffer) 
{
	// registers[0] -> eax
	// registers[1] -> ebx
	// registers[2] -> ecx
	// registers[3] -> edx
	int registers[4];

	int cpuid_addr = 0x80000002;
	
    for(int i = 0; i < 3; i++) 
    {
    	cpuid_addr += i;
        __asm__("cpuid" : "=a"(registers[0]), "=b"(registers[1]), "=c"(registers[2]), "=d"(registers[3]) : "a"(cpuid_addr));
        
        for(int j = 0; j < 4; j++)
            ((int*)(outCpuInfoBuffer + i * 16))[j] = registers[j];
    }

    outCpuInfoBuffer[48] = '\0';
}
