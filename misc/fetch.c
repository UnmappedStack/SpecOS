/* Fetch command to celebrate 100 stars on GitHub!
 * Copyright (c) 2024 Jake Steinburger under the MIT license. See the GitHub repo for more information.
 */

#include <cpuid.h>
#include "../limine.h"
#include "fetch.h"
#include "fetchicon.h"
#include "../kernel/include/kernel.h"
#include "../drivers/include/vga.h"
#include "../utils/include/printf.h"

// these CPUID string definitions are from the osdev wiki.
// Vendor strings from CPUs.
#define CPUID_VENDOR_AMD           "AuthenticAMD"
#define CPUID_VENDOR_AMD_OLD       "AMDisbetter!" // Early engineering samples of AMD K5 processor
#define CPUID_VENDOR_INTEL         "GenuineIntel"
#define CPUID_VENDOR_VIA           "VIA VIA VIA "
#define CPUID_VENDOR_TRANSMETA     "GenuineTMx86"
#define CPUID_VENDOR_TRANSMETA_OLD "TransmetaCPU"
#define CPUID_VENDOR_CYRIX         "CyrixInstead"
#define CPUID_VENDOR_CENTAUR       "CentaurHauls"
#define CPUID_VENDOR_NEXGEN        "NexGenDriven"
#define CPUID_VENDOR_UMC           "UMC UMC UMC "
#define CPUID_VENDOR_SIS           "SiS SiS SiS "
#define CPUID_VENDOR_NSC           "Geode by NSC"
#define CPUID_VENDOR_RISE          "RiseRiseRise"
#define CPUID_VENDOR_VORTEX        "Vortex86 SoC"
#define CPUID_VENDOR_AO486         "MiSTer AO486"
#define CPUID_VENDOR_AO486_OLD     "GenuineAO486"
#define CPUID_VENDOR_ZHAOXIN       "  Shanghai  "
#define CPUID_VENDOR_HYGON         "HygonGenuine"
#define CPUID_VENDOR_ELBRUS        "E2K MACHINE "
 
// Vendor strings from hypervisors.
#define CPUID_VENDOR_QEMU          "TCGTCGTCGTCG"
#define CPUID_VENDOR_KVM           " KVMKVMKVM  "
#define CPUID_VENDOR_VMWARE        "VMwareVMware"
#define CPUID_VENDOR_VIRTUALBOX    "VBoxVBoxVBox"
#define CPUID_VENDOR_XEN           "XenVMMXenVMM"
#define CPUID_VENDOR_HYPERV        "Microsoft Hv"
#define CPUID_VENDOR_PARALLELS     " prl hyperv "
#define CPUID_VENDOR_PARALLELS_ALT " lrpepyh vr " // Sometimes Parallels incorrectly encodes "prl hyperv" as "lrpepyh vr" due to an endianness mismatch.
#define CPUID_VENDOR_BHYVE         "bhyve bhyve "
#define CPUID_VENDOR_QNX           " QNXQVMBSQG "

char* getCPU() {
    // may not work on CPUs that don't support it. 
    // Oh well, fuck 'em.
    uint32_t ebx, ecx, edx;
    asm volatile("cpuid"
        : "=b" (ebx), "=c" (ecx), "=d" (edx)
        : "a"(0));
    static char toReturn[14];
    toReturn[13] = 0;
    *(uint32_t*)(toReturn    ) = ebx;
    *(uint32_t*)(toReturn + 4) = edx;
    *(uint32_t*)(toReturn + 8) = ecx;
    return toReturn;
}

void showFetch() {
    // get total memory
    struct limine_memmap_entry **memmapEntries = kernel.memmapEntries;
    uint64_t memmapEntryCount = kernel.memmapEntryCount;
    uint64_t memSize = memmapEntries[memmapEntryCount - 2]->base + memmapEntries[memmapEntryCount - 2]->length;
    writestring("\n"); 
    // draw text
    char* gap = "       "; // 7 spaces
    int memSizeMB = memSize / 10000000; // seems like too many zeros? for some reason it needs this amount.
    printf("%sMemory  %imb\n", gap, memSizeMB);
    printf("%sKernel  SpecOS nucleus 2024, open beta\n", gap);
    printf("%sCPU     %s\n", gap, getCPU());
    // draw image
    kernel.chX = 5;
    kernel.chY -= 48;
    for (int x = 0; x < 50; x++) {
        for (int y = 0; y < 50; y++) {
            drawPix(kernel.chX + x, kernel.chY + y, fetchIcon[y * 50 + x]);
        }
    }
    writestring("\n\n\n\n");
}
