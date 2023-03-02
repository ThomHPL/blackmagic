#include "command.h"
#include "general.h"
#include "target.h"
#include "target_internal.h"
#include "cortexm.h"
#include "adiv5.h"
#include <stdio.h>

#define APOLLO3B_CHIPPN 0x40020000

#define APOLLO3B_FLASH_BASE 0x00000000
#define APOLLO3B_FLASH_SIZE 0x00100000
#define APOLLO3B_BLOCKSIZE  2048

#define APOLLO3B_TCM_SRAM_START  0x10000000
#define APOLLO3B_TCM_SRAM_LEN    0x00010000
#define APOLLO3B_MAIN_SRAM_START 0x10010000
#define APOLLO3B_MAIN_SRAM_LEN   0x00050000

#define CMD_PROGRAM_MAIN_FLASH      0x0800005D
#define CMD_PROGRAM_INFO0_FLASH     0x08000061
#define CMD_FLASH_ERASE_PAGES_MAIN  0x08000065
#define CMD_FLASH_MASS_ERASE_MAIN   0x08000069
#define CMD_UNKNOWN_1               0x080000CA // PC when entering debug

#define PROGRAM_KEY         (0x12344321)
#define INFO0_PROGRAM_KEY   (0x87655678)

static int ambiq_run_cmd(target *t, uint32_t command, uint32_t flash_return_address) {
    (void) flash_return_address;
    (void) command;

    DEBUG_INFO("Run command at 0x%8X\n", command);

    uint32_t backup_regs[t->regs_size / sizeof(uint32_t)];
	target_regs_read(t, backup_regs);
    backup_regs[10] = 0x1;
    t->regs_write(t, backup_regs);
    target_mem_write32(t, CORTEXM_DCRDR, command);
    target_mem_write32(t, CORTEXM_DCRSR, CORTEXM_DCRSR_REGWnR | 0b0001100);
    // target_mem_write32(t, CORTEXM_DHCSR, CORTEXM_DHCSR_DBGKEY | CORTEXM_DHCSR_C_MASKINTS | CORTEXM_DHCSR_C_STEP | CORTEXM_DHCSR_C_DEBUGEN);

    return true;
}

static bool set_core_reg(target *t, int argc, const char *argv[]) {
    if (argc != 3) {
        DEBUG_INFO("Bad number of arguments: %d\n", argc);
        return false;
    }
    DEBUG_INFO("%s | %s | %s\n", argv[0], argv[1], argv[2]);
    
    uint32_t num = (uint32_t) strtol(argv[1], NULL, 10);
    uint32_t value = (uint32_t) strtol(argv[2], NULL, 16);

    target_mem_write32(t, CORTEXM_DCRDR, value);
    DEBUG_INFO("DCRDR written to 0x%lX\n", target_mem_read32(t, CORTEXM_DCRDR));
  
    target_mem_write32(t, CORTEXM_DCRSR, num | CORTEXM_DCRSR_REGWnR);
    DEBUG_INFO("DCRSR written to 0x%lX\n", target_mem_read32(t, CORTEXM_DCRSR));
    target_mem_write32(t, CORTEXM_DCRDR, 0);

    target_mem_write32(t, CORTEXM_DCRSR, num);
    DEBUG_INFO("Register %ld has value 0x%lX\n", num, target_mem_read32(t, CORTEXM_DCRDR));
    return true;
}

static bool get_core_reg(target *t, int argc, const char *argv[]) {
    if (argc != 2) {
        DEBUG_INFO("Bad number of arguments: %d\n", argc);
        return false;
    }
    DEBUG_INFO("%s | %s | %s\n", argv[0], argv[1], argv[2]);
    
    uint32_t num = (uint32_t) strtol(argv[1], NULL, 10);

    target_mem_write32(t, CORTEXM_DCRSR, num);
    DEBUG_INFO("Register %ld has value 0x%lX\n", num, target_mem_read32(t, CORTEXM_DCRDR));
    return true;
}

static bool program_main_flash(target *t, int argc, const char *argv[]) {

    if (argc != 3) {
        DEBUG_INFO("Bad number of arguments: %d\n", argc);
        return false;
    }
    DEBUG_INFO("%s | %s | %s\n", argv[0], argv[1], argv[2]);
    
    uint32_t count = (uint32_t) strtol(argv[1], NULL, 16);
    uint32_t offset = (uint32_t) strtol(argv[2], NULL, 16);

    if (offset < 0x00010000) {
        DEBUG_INFO("Destination address is in bootloader\n");
    }

    DEBUG_INFO("Trying to write %ld bytes @ 0x%lX\n", count, offset);
    if (((count%4) != 0) || ((offset%4) != 0)) {
        DEBUG_INFO("write block must be multiple of 4 bytes in offset & length\n");
        return false;
    }

    // Pointer to flash
    target_mem_write32(t, 0x10000000, offset);

    // Number of 32-bit words to program
    target_mem_write32(t, 0x10000004, count/4);

    // Write Key
    target_mem_write32(t, 0x10000008, PROGRAM_KEY);

    // Breakpoint
    target_mem_write32(t, 0x1000000C, 0xFFFFFFFF);
    
    // Write data to SRAM
    uint32_t data[] = {0x1005FFF4};
    target_mem_write(t, 0x10001000, (uint8_t*) data, 4);

    ambiq_run_cmd(t, CMD_PROGRAM_MAIN_FLASH, 0x1000000C);

    // clear bootloader bit
    // target_mem_write32(t, 0x400201a0, 0x0);
    return true;
}

 
//  static int ambiqmicro_write_block(struct flash_bank *bank,
    //  const uint8_t *buffer, uint32_t offset, uint32_t count)
//  {
     /* struct ambiqmicro_flash_bank *ambiqmicro_info = bank->driver_priv; */
    //  struct target *target = bank->target;
    //  uint32_t address = bank->base + offset;
    //  uint32_t buffer_pointer = 0x10000010;
    //  uint32_t maxbuffer;
    //  uint32_t thisrun_count;
    // int retval = 0;
  

  
    //  LOG_INFO("Flashing main array");
  
    //  while (count > 0) {
        //  if (count > maxbuffer)
            //  thisrun_count = maxbuffer;
        //  else
            //  thisrun_count = count;
  
         /*
          * Set up the SRAM.
          */
  
         /*
          * Pointer to flash.
          */
        //  target_mem_write32(target, 0x10000000, address);
         //CHECK_STATUS(retval, "error writing target SRAM parameters.");
  
         /*
          * Number of 32-bit words to program.
          */
        //  target_mem_write32(target, 0x10000004, thisrun_count/4);
         //CHECK_STATUS(retval, "error writing target SRAM parameters.");
  
         /*
          * Write Key.
          */
        //  target_mem_write32(target, 0x10000008, PROGRAM_KEY);
         //CHECK_STATUS(retval, "error writing target SRAM parameters.");
  
         /*
          * Breakpoint.
          */
        //  target_mem_write32(target, 0x1000000c, 0xfffffffe);
         //CHECK_STATUS(retval, "error writing target SRAM parameters.");
  
         /*
          * Write Buffer.
          */
        //  target_write_buffer(target, buffer_pointer, thisrun_count, buffer);
  
        //  if (retval != ERROR_OK) {
        //      CHECK_STATUS(retval, "error writing target SRAM parameters.");
        //      break;
        //  }
  
        //  LOG_DEBUG("address = 0x%08" PRIx32, address);
  
        //  retval = ambiqmicro_exec_command(target, FLASH_PROGRAM_MAIN_FROM_SRAM, 0x1000000c);
        //  CHECK_STATUS(retval, "error executing ambiqmicro flash write algorithm");
        //  if (retval != ERROR_OK)
        //      break;
        //  buffer += thisrun_count;
        //  address += thisrun_count;
        //  count -= thisrun_count;
    //  }
  
  
    //  LOG_INFO("Main array flashed");
  
    //  /*
    //   * Clear Bootloader bit.
    //   */
    //  retval = target_mem_write32(target, 0x400201a0, 0x0);
    //  CHECK_STATUS(retval, "error clearing bootloader bit");
  
    //  return retval;
//  }

static bool program_otp_flash(target *t, int argc, const char *argv[]) {
    (void) argc;
    (void) argv;
    (void) t;
    return true;
}

static bool erase_pages_main(target *t, int argc, const char *argv[]) {
    (void) argc;
    (void) argv;
    (void) t;
    return true;
}

static bool mass_erase_main(target *t, int argc, const char *argv[]) {
    (void) argc;
    (void) argv;
    (void) t;
    return true;
}

const struct command_s apollo3b_cmd_list[] = {
	{"program_main_flash", program_main_flash, "Program the main flash from the SRAM"},
    {"program_OTP_flash", program_otp_flash, "Program the OTP flash from the SRAM"},
    {"erase_pages_main", erase_pages_main, "Erase specific pages from main flash"},
    {"mass_erase_main", mass_erase_main, "Mass erase the whole flash"},
    {"set_core_reg", set_core_reg, "Set core register to a specific value"},
    {"get_core_reg", get_core_reg, "Get core register"},
	{NULL, NULL, NULL}
};

static int apollo3b_flash_erase(){//struct target_flash *f, target_addr addr, size_t len) {
    return 0;
}
static int apollo3b_flash_write(struct target_flash *f, target_addr dest, const void *src, size_t len) {
    (void) f;
    (void) src;

    DEBUG_INFO("Trying to write %d bytes @ 0x%8lX\n", len, dest);

    if (((len%4) != 0) || ((dest%4) != 0)) {
         DEBUG_INFO("write block must be multiple of 4 bytes in offset & length\n");
         return false;
     }
    return 0;
}

static void apollo3b_add_flash(target *t) {
    struct target_flash *f = calloc(1, sizeof(*f));
    f->start = APOLLO3B_FLASH_BASE;
    f->length = APOLLO3B_FLASH_SIZE;
    f->blocksize = APOLLO3B_BLOCKSIZE;
    f->erase = apollo3b_flash_erase;
    f->write = apollo3b_flash_write;
    target_add_flash(t, f);
}

bool apollo3b_probe(target *const t) {
    uint32_t chipid = target_mem_read32(t, APOLLO3B_CHIPPN);
    
    DEBUG_INFO("Ambiq Apollo Chip IP: %8lX\n",chipid);
    if (((chipid & 0xFF000000) >> 0x18) == 0x06) {
        t->driver = "Apollo3 Blue";
        apollo3b_add_flash(t);
        target_add_ram(t, APOLLO3B_TCM_SRAM_START, APOLLO3B_TCM_SRAM_LEN);
        target_add_ram(t, APOLLO3B_MAIN_SRAM_START, APOLLO3B_MAIN_SRAM_LEN);
        target_add_commands(t, apollo3b_cmd_list, "Apollo3 Blue");

        return true;
    }
    return false;   
}