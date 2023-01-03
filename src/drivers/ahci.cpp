#include "drivers/ahci.hpp"
#include "drivers/pci.hpp"
#include "drivers/ps2.hpp"
#include "paging.hpp"
#include "kernel.hpp"
#include "heap.hpp"
#include "isr.hpp"

#define AHCI_VS_0_95  0x00000905
#define AHCI_VS_1_0   0x00010000
#define AHCI_VS_1_1   0x00010100
#define AHCI_VS_1_2   0x00010200
#define AHCI_VS_1_3   0x00010300
#define AHCI_VS_1_3_1 0x00010301

namespace kahci
{
    hba_mem* hba_memory;
    static port_cmd_awaiter awaiting_ports[32];

    const uint32 INT_STATUS_FATAL_ERR_MASK = 0x78000000;

    static void handle_int_port(int port_num, uint32 int_status);
    static void ahci_int_handler(kisr::regs_t regs);
    static int find_free_cmd_slot(hba_port* port);

    static void initialize_port(hba_port* port);

    void init()
    {
        kpci::pci_device* ahci = kpci::find_device(0x01, 0x06);
        if(ahci == nullptr)
        {
            kernel_print_log("[AHCI] WARNING: Controller not found in PCI device list.\n");
            return;
        }

        uint32 hba_addr = ahci->read(0x24) & 0xFFFFF800;
        hba_memory = (hba_mem*) hba_addr;
        kpaging::map_address(hba_addr);

        // Register AHCI handler
        kisr::register_handler(IRQ_BASE + 10, ahci_int_handler);

        // Enable AHCI-only, reset HBA controller and wait until it resets
        hba_memory->ghc |= 1 << 31;
        hba_memory->ghc |= 1;
        while(hba_memory->ghc & 1)
        {
            asm volatile("hlt");
        }

        // Enable AHCI-only mode again
        hba_memory->ghc |= 1 << 31;
        
        // Determine which ports is implemented
        uint32 imp_ports = kahci::hba_memory->pi;
        for(int i = 0; i < 32; i++)
        {
            // Port is implemented?
            if(imp_ports & 1)
            {
                hba_port* port = &hba_memory->ports[i];

                printf("[AHCI] Prepairing implemented port #%d\n", i);
                initialize_port(port);
            }

            imp_ports >>= 1;
        }

        // Clear global interrupt bits
        hba_memory->is = 0xFFFFFFFF;
        // Enabling all types of interrupts for the entire controller
        hba_memory->ghc |= 2;

        kernel_print_log("AHCI driver init completed.\n");
    }

    string get_version()
    {
        if(hba_memory == nullptr)
            return "AHCI NOT INITED";

        switch(hba_memory->vs)
        {
            case AHCI_VS_0_95:
                return "0.95";
            case AHCI_VS_1_0:
                return "1.0";
            case AHCI_VS_1_1:
                return "1.1";
            case AHCI_VS_1_2:
                return "1.2";
            case AHCI_VS_1_3:
                return "1.3";
            case AHCI_VS_1_3_1:
                return "1.3.1";

            default:
                return "Unknown";
        }
    }

    bool read(int port_num, uint32 start_low, uint32 start_high, uint32 count, uint16* buffer)
    {
        if(hba_memory == nullptr) return false;
        hba_port* port = &hba_memory->ports[port_num];

        // Limited because we have only 8 entries in hba_cmd_table
        if(count > 1024 * 64)
            return false;

        int free_slot = find_free_cmd_slot(port);
        if(free_slot == -1)
            return false;

        // Select right command slot
        hba_cmd_hdr* cmd_header = (hba_cmd_hdr*) port->clb;
        cmd_header += free_slot;

        // Map address where the command slot located
        kpaging::map_address((uint32) cmd_header);

        cmd_header->cfl = sizeof(fis_reg_h2d_t) / sizeof(uint32);
        cmd_header->w = 0;
        cmd_header->prdtl = (uint16) ((count - 1) / 16) + 1;

        // Get command table from the slot and map address where it located
        hba_cmd_table* cmd_table = (hba_cmd_table*) cmd_header->ctba;
        kpaging::map_address((uint32) cmd_table);

        // Clear command table structure
        mem_fill((uint8*) cmd_table, 0, sizeof(hba_cmd_table));

        // Fill PRDT
        int i = 0;
        for( ; i < cmd_header->prdtl - 1; i++)
        {
            cmd_table->prdt_entry[i].dba = (uint32) buffer;
            cmd_table->prdt_entry[i].dbc = 1024 * 8 - 1;
            cmd_table->prdt_entry[i].i = true;

            buffer += 1024 * 4;
            count -= 16;
        }

        // Last entry
        cmd_table->prdt_entry[i].dba = (uint32) buffer;
        cmd_table->prdt_entry[i].dbc = (count * 512) - 1;
        cmd_table->prdt_entry[i].i = true;

        // Build host to device FIS 
        fis_reg_h2d_t* cmd_fis = (fis_reg_h2d_t*) &cmd_table->cfis;
        cmd_fis->fis_type = fis_type::FIS_TYPE_REG_H2D;
        cmd_fis->c = 1;
        cmd_fis->command = 0x24;

        cmd_fis->lba0 = (uint8) start_low;
        cmd_fis->lba1 = (uint8) (start_low >> 8);
        cmd_fis->lba2 = (uint8) (start_low >> 16);
        cmd_fis->device = 1 << 6;

        cmd_fis->lba3 = (uint8) (start_low >> 24);
        cmd_fis->lba4 = (uint8) start_high;
        cmd_fis->lba5 = (uint8) (start_high >> 8);

        cmd_fis->countl = count & 0xFF;
        cmd_fis->counth = (count >> 8) & 0xFF;

        // Wait until port will be free to issue command
        int spin = 0;
        uint8 busy_or_drq = 0b10001000;
        while((port->tfd & busy_or_drq) && spin < 1000000)
        {
            spin++;
        }

        if(spin == 1000000)
        {
            printf("[AHCI] Port is hung\n");
            return false;
        }

        // Issue command on selected slot
        port->ci = 1 << free_slot;

        port_cmd_awaiter* awaiter = &awaiting_ports[port_num];
        awaiter->commands[free_slot].status = awaiter_status::WAITING;

        // Wait until command will be processed
        while(true)
        {
            asm volatile("hlt");

            // If the command isn't completed
            if(awaiter->commands[free_slot].status != awaiter_status::COMPLETED)
                continue;

            awaiter->commands[free_slot].status = awaiter_status::IDLE;

            // If success
            if(awaiter->commands[free_slot].success)
                return true;

            // If any error
            printf("[AHCI] Read error\n");
            return false;
        }
    }

    static int find_free_cmd_slot(hba_port* port)
    {
        uint32 slots = port->sact | port->ci;
        for(int i = 0; i < 32; i++)
        {
            if((slots & 1) == false)
                return i;
            
            slots >>= 1;
        }

        printf("[AHCI] Free command slot not found\n");
        return -1;
    }

    port_devtype get_port_devtype(hba_port* port)
    {
        if(hba_memory == nullptr || port->ssts.det != 3 || port->ssts.ipm != 1)
        {
            return NOT_PRESENT;
        }

        switch(port->sig)
        {
            case 0xEB140101:
                return DEV_SATAPI;
            case 0xC33C0101:
                return DEV_SEMB;
            case 0x96690101:
                return DEV_PM;

            default:
                return DEV_SATA;
        }
    }

    static void ahci_int_handler(kisr::regs_t regs)
    {
        // Get ports that caused the interrupt
        uint32 int_ports = hba_memory->is;

        // Enumerating all ports that caused the interrupt
        uint32 ports_to_enum = int_ports;
        for(int i = 0; i < 32; i++)
        {
            if(ports_to_enum & 1)
            {
                hba_port* port = &hba_memory->ports[i];
                uint32 int_status = port->is;
				uint32 s_err = port->serr;

                // printf("[AHCI] Interrupt status (0x%x) from port #%d\n", int_status, i);
                handle_int_port(i, int_status);

				port->serr = s_err;
                port->is = int_status;
            }
            
            ports_to_enum >>= 1;
        }

        // Clear the interrupts
        hba_memory->is = int_ports;
    }

    static void handle_int_port(int port_num, uint32 int_status)
    {
        hba_port* port = &hba_memory->ports[port_num];
        port_cmd_awaiter* awaiter = &awaiting_ports[port_num];

        // Notify all waiters about command results
        for(int i = 0; i < 32; i++)
        {
            if(awaiter->commands[i].status != awaiter_status::WAITING) continue;
            if(port->ci & (1 << i)) continue;

            awaiter->commands[i].success = int_status & INT_STATUS_FATAL_ERR_MASK ? false : true;
            awaiter->commands[i].status = awaiter_status::COMPLETED;
        }

        // Continue only if fatal error occured
        if((int_status & INT_STATUS_FATAL_ERR_MASK) == false) return;
        printf("[AHCI] Fatal error occured (IS = %x)\n", int_status);

        // Disable command list processing and wait until it's disabled
        port->cmd &= ~1;
        while(port->cmd & (1 << 15));

        // Clearing errors
        port->serr = 0xFFFFFFFF;

        uint8 busy_or_drq = 0b10001000;
        if(port->tfd & busy_or_drq)
        {
            // Sending COMRESET
            port->sctl.det = 1;
            asm volatile("hlt");
            asm volatile("hlt");
            port->sctl.det = 0;

            // Waiting for communication re-establishing
            while(port->ssts.det != 3)
                asm volatile("hlt");
        }

        // Enable command list processing
        port->cmd |= 1;
    }

    static void initialize_port(hba_port* port)
    {
        // Command processing already enabled?
        if(port->cmd & 1) return;

        // No device present on the port?
        if(port->ssts.det != 3) return;

        // Clear error bits
        port->serr = 0xFFFFFFFF;
        // Enable FRE, so we can receive FISes
        port->cmd |= 1 << 4;
        
        // Wait for BSY and DRQ will be cleared
        // Trying to set ST bit when interface is busy = can cause problems
        uint8 busy_or_drq_mask = 0b10001000;
        while(port->tfd & busy_or_drq_mask)
        {
            asm volatile("hlt");
        }

        // Wait until command engine isn't running
        while(port->cmd & (1 << 15))
        {
            asm volatile("hlt");
        }

        // Finally enable command engine
        port->cmd |= 1;

        // Clear interrupt bits, or invalid interrupt calls may be executed
        port->is = 0xFFFFFFFF;
        // Enabling all interrupts, we handle everything
        port->ie = 0xFFFFFFFF;
    }
}
