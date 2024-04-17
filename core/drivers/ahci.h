#pragma once
#include "pci.h"

namespace kahci
{
    /*
        AHCI related stuff 
        If you want to know what the structures below means - read the AHCI spec.
    */

    /* HBA Port Registers */
    struct hba_port
    {
        /* Command List Base Address */
        uint32 clb;
        /* Command List Base Address Upper 32-bits */
        uint32 clbu;
        /* Port x FIS Base Address */
        uint32 fb;
        /* FIS Base Address Upper 32-bits */
        uint32 fbu;
        /* Interrupt Status */
        uint32 is;
        /* Interrupt Enable */
        uint32 ie;
        /* Command and Status */
        uint32 cmd;
        uint32 reserved;
        /* Task File Data */
        uint32 tfd;
        /* Signature */
        uint32 sig;

        /* Serial ATA Status (SCR0: SStatus) */
        struct
        {
            uint8 det : 4;
            uint8 spd : 4;
            uint8 ipm : 4;
            uint32 reserved : 20;
        } __attribute__((packed)) ssts;

        /* Serial ATA Control (SCR2: SControl) */
        struct
        {
            uint8 det : 4;
            uint8 spd : 4;
            uint8 ipm : 4;
            uint8 spm : 4;
            uint8 pmp : 4;
            uint16 reserved : 12;
        } __attribute__((packed)) sctl;

        /* Serial ATA Error (SCR1: SError) */
        uint32 serr;
        /* Serial ATA Active (SCR3: SActive) */
        uint32 sact;
        /* Command Issue */
        uint32 ci;
        /* Serial ATA Notification (SCR4: SNotification) */
        uint32 sntf;
        /* FIS-based Switching Control */
        uint32 fbs;

        uint32 reserved2[11];
        /* Vendor Specific Registers */
        uint32 vendor[4];
    } __attribute__((packed));

    /* HBA Memory Registers */
    struct hba_mem
    {
        /* HBA Capabilities */
        uint32 cap;
        /* Global HBA Control */
        uint32 ghc;
        /* Interrupt Status Register */
        uint32 is;
        /* Ports Implemented */
        uint32 pi;
        /* AHCI Version */
        uint32 vs;
        /* Command Completion Coalescing Control */
        uint32 ccc_ctl;
        /* Command Completion Coalescing Ports */
        uint32 ccc_pts;
        /* Enclosure Management Location */
        uint32 em_loc;
        /* Enclosure Management Control */
        uint32 em_ctl;
        /* HBA Capabilities Extended */
        uint32 cap2;
        /* BIOS/OS Handoff Control and Status */
        uint32 bohc;
        
        uint8 reserved[0xA0-0x2C];
        
        /* Vendor Specific Registers */
        uint8 vendor_regs[0x100-0xA0];
        /* Port Registers */
        hba_port ports[32];
    } __attribute__((packed));

    struct hba_cmd_hdr
    {
        uint8 cfl : 5;
        uint8 a : 1;
        uint8 w : 1;
        uint8 p : 1;
        uint8 r : 1;
        uint8 b : 1;
        uint8 c : 1;
        uint8 reserved : 1;
        uint8 pmp : 4;
        uint16 prdtl;

        uint32 prdbc;
        uint32 ctba;
        uint32 ctbau;
        uint32 reserved2[4];
    } __attribute__((packed));

    struct hba_prdt_entry
    {
        uint32 dba;
        uint32 dbau;
        uint32 reserved;

        uint32 dbc:22; 
        uint32 reserved2 : 9;
        uint32 i : 1;
    } __attribute__((packed));

    struct hba_cmd_table
    {
        uint8 cfis[64];
        uint8 acmd[16];
        uint8 reserved[48];

        hba_prdt_entry prdt_entry[8];
    } __attribute__((packed));

    /*
        SATA related stuff
        If you want to know what the structures below means - read the SATA spec.
    */

    enum fis_type
    {
        FIS_TYPE_REG_H2D = 0x27,
        FIS_TYPE_REG_D2H = 0x34,
        FIS_TYPE_DMA_ACT = 0x39,
        FIS_TYPE_DMA_SETUP = 0x41,
        FIS_TYPE_DATA = 0x46,
        FIS_TYPE_BIST = 0x58,
        FIS_TYPE_PIO_SETUP = 0x5F,
        FIS_TYPE_DEV_BITS = 0xA1,
    };

    struct fis_reg_h2d_t
    {
        uint8 fis_type;
        uint8 pmport:4;
        uint8 reserved:3;
        uint8 c:1;
        uint8 command;
        uint8 featurel;

        uint8 lba0;
        uint8 lba1;
        uint8 lba2;
        uint8 device;

        uint8 lba3;
        uint8 lba4;
        uint8 lba5;
        uint8 featureh;

        uint8 countl;
        uint8 counth;
        uint8 icc;
        uint8 control;

        uint8 reserved2[4];
    } __attribute__((packed));

    /*
        Kernel lib related stuff
    */

    enum port_devtype
    {
        NOT_PRESENT,
        DEV_SATAPI,
        DEV_SEMB,
        DEV_PM,
        DEV_SATA
    };

    enum awaiter_status
    {
        IDLE,
        WAITING,
        COMPLETED
    };

    struct port_cmd_awaiter
    {
        struct
        {
            awaiter_status status : 2;
            bool success : 1;
        } __attribute__((packed)) commands[32];
    } __attribute__((packed));

    extern hba_mem* hba_memory;

    /* Init the driver */
    void init();
    /* Get AHCI controller version */
    string get_version();
    /* Get device type that connected to specified port */
    port_devtype get_port_devtype(hba_port* port);

    /* Read data from given port to specific location */
    bool read(int port_num, uint32 start_low, uint32 start_high, uint32 count, uint16* buffer);
}
