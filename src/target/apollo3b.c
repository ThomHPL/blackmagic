#include "command.h"
#include "general.h"
#include "target.h"
#include "target_internal.h"
#include "adiv5.h"
#include <stdio.h>

#define APOLLO3B_CHIPPN 0x40020000

#define APOLLO3_FLASH_BASE 0x00000000
#define APOLLO3_FLASH_SIZE 0x00100000
#define APOLLO3_BLOCKSIZE 2048

#define APOLLO3_TCM_SRAM_START  0x10000000
#define APOLLO3_TCM_SRAM_LEN    0x00010000
#define APOLLO3_MAIN_SRAM_START 0x10010000
#define APOLLO3_MAIN_SRAM_LEN   0x00050000

static int apollo3_flash_erase(){//struct target_flash *f, target_addr addr, size_t len) {
    return 0;
}
static int apollo3_flash_write(){//struct target_flash *f, target_addr dest, const void *src, size_t len) {
    return 0;
}

static void apollo3_add_flash(target *t) {
    struct target_flash *f = calloc(1, sizeof(*f));
    f->start = APOLLO3_FLASH_BASE;
    f->length = APOLLO3_FLASH_SIZE;
    f->blocksize = APOLLO3_BLOCKSIZE;
    f->erase = apollo3_flash_erase;
    f->write = apollo3_flash_write;
    target_add_flash(t, f);
}

bool apollo3b_probe(target *const t) {
    uint32_t chipid;

	chipid = target_mem_read32(t, APOLLO3B_CHIPPN);
    
    DEBUG_INFO("Chip IP: %8lX\n",chipid);

    t->driver = "Apollo3 Blue";
    
    apollo3_add_flash(t);
    target_add_ram(t, APOLLO3_TCM_SRAM_START, APOLLO3_TCM_SRAM_LEN);
    target_add_ram(t, APOLLO3_MAIN_SRAM_START, APOLLO3_MAIN_SRAM_LEN);

    return true;
}