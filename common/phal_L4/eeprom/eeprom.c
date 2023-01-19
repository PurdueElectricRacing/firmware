#if FALSE
#include "common/phal_L4/eeprom/eeprom.h"

// EEPROM struct
struct eeprom mem;
uint8_t       mem_zero[MICRO_PG_SIZE];

// Static prototypes
static int  readMem(uint16_t phys_addr, uint8_t* loc_addr, uint16_t len);
static int  writePage(uint16_t addr, uint8_t* page, uint8_t size);
static int  readPage(uint16_t addr, uint8_t* page);
static void memClear(void);
static int  memTest(void);
static int  fnameSearch(char* name);
static int  ee_memcheck(uint8_t* src, uint8_t* dest, size_t len);
static void ee_memset(uint8_t* dest, size_t len, uint8_t value);
static void ee_memcpy(uint8_t* src, uint8_t* dest, size_t len);

// @funcname: initMem
//
// @brief: Initializes chip metadata. Attemps to read
//         and load all metadata from last run. If memory
//         isn't initialized, force_init can be set to 
//         set a default version of 1 and mark the chip
//         as officially in use
//
// @param: wc_gpio_port: Mem lock port0
// @param: wc_gpio_pin: Mem lock pin
// @param: version: Version of app code
// @param: force_init: Force initialization if memory isn't
//                     initialized. Only set if version is truly 1
int initMem(GPIO_TypeDef* wc_gpio_port, uint32_t wc_gpio_pin, uint16_t version, bool force_init) {
    int      ret;
    uint16_t i, size, end;
    uint8_t  page[MICRO_PG_SIZE];

    // Set WC pins
    mem.wc_gpio_port = wc_gpio_port;
    mem.wc_gpio_pin = wc_gpio_pin;

    // Read EEPROM metadata
    ret = readMem(0, (uint8_t*) &mem.phys, sizeof(struct phys_mem));
    ee_memset(mem_zero, MICRO_PG_SIZE, 0);

    // Ensure there are no errors
    if (ret < 0) {
        return ret;
    }

    // Check if the memory isn't initialized
    if (!mem.phys.init) {
        // It isn't, and we want it to be, so set to default values
        if (force_init) {
            mem.phys.init = true;
            mem.phys.version = 1;
            mem.init_physical = true;

            return E_SUCCESS;
        } else {
            return -E_NO_INIT;
        }
    }

    // Check if we have a version match
    ret = checkVersion(version);

    // Return error code if version is off
    if (ret < 0) {
        return -E_V_MISMATCH;
    }

    // Start loading process
    ee_memset(page, NAME_LEN, 0);

    for (i = 0; i < MAX_PAGES; i++) {
        if (ee_memcheck(page, (char*) &mem.phys.filename[i], NAME_LEN) < 0) {
            ret = readMem(mem.phys.pg_bound[i], mem.pg_addr[i], mem.pg_size[i]);

            if (ret < 0) {
                return ret;
            }
        }
    }

    mem.init_physical = true;

    return E_SUCCESS;
}

// @funcname: checkVersion
//
// @brief: Checks version to ensure app code is new enough
//
// @param: dest: Pointer to location to set
// @param: len: Length to set
// @param: value: Value to set each memory address to
// @param: Difference between current and chip versions
//         if app code is newer, -E_V_MISMATCH if app
//         code is old. -E_NO_INIT if memory hasn't
//         been initialized yet
int checkVersion(uint16_t version) {
    // Check if we're even initialized
    if (!mem.phys.init) {
        return -E_NO_INIT;
    }

    // Check if our software is a later version
    if (mem.phys.version < version) {
        return -E_V_MISMATCH;
    } else {
        return mem.phys.version - version;
    }
}

// @funcname: mapMem
//
// @brief: Maps local address to chip address
//
// @param: addr: Pointer to local address
// @param: len: Length of data
// @param: fname: File name (NAME_LEN characters)
// @param: bcmp: Backwards compatibility enabled. Disable for temp storage
int mapMem(uint8_t* addr, uint16_t len, uint8_t* fname, bool bcmp) {
    int     i;
    uint8_t null_name[NAME_LEN];

    ee_memset(null_name, NAME_LEN, 0U);

    // Check if we already found this memory on EEPROM
    i = fnameSearch(fname);

    if (i >= 0) {
        mem.pg_addr[i] = addr;
        mem.pg_size[i] = len;

        return E_SUCCESS;
    }

    // Check for the first free location
    for (i = 0; i < MAX_PAGES; i++) {
        if (ee_memcheck(null_name, &mem.phys.filename[i * NAME_LEN], NAME_LEN) == 0) {
            break;
        }
    }

    // Check if we found a free location
    if (i == MAX_PAGES) {
        return -E_NO_MEM;
    }

    // Copy over new metadata
    ee_memcpy(fname, &mem.phys.filename[i * NAME_LEN], NAME_LEN);
    mem.phys.mem_size[i] = len;
    mem.phys.bcmp[i] = bcmp;
    mem.phys.pg_bound[i] = (i == 0) ? MACRO_PG_SIZE : ROUNDUP(mem.phys.pg_bound[i - 1] + mem.phys.mem_size[i - 1], MICRO_PG_SIZE);
    mem.pg_addr[i] = addr;
    mem.pg_size[i] = len;

    return E_SUCCESS;
}

// @funcname: memBg
//
// @brief: Background task for searching for stale data
//         If metadata or any mapped struct is updated,
//         it will be written to the device once searched
//         by the foreground loop
//
// @note: Application code must add to background queue with rate MEM_FG_TIME
void memBg(void) {
    int      ret;
    uint8_t  page[MICRO_PG_SIZE];
    uint16_t len, end;

    static uint8_t  search;
    static uint32_t addr;

    // Check if we're waiting on foreground to execute a write
    if (mem.write_pending) {
        return;
    }

    // Check if we want to zero out the EEPROM
    if (mem.zero_req) {
        mem.write_pending = true;
        mem.source_loc = mem_zero;
        mem.dest_loc = addr;
        mem.update_len = MICRO_PG_SIZE;
        addr += MICRO_PG_SIZE;

        if (addr >= CHIP_SIZE) {
            mem.zero_req = false;
            addr = 0;
            search = 0;
        }

        return;
    }

    // Check if we want a search of metadata. Else, search known locations
    if (search == 0) {
        end = ROUNDUP(sizeof(struct phys_mem), MICRO_PG_SIZE);
        len = MICRO_PG_SIZE - (end % MICRO_PG_SIZE);
        ret = readMem(addr, page, len);

        if (ret < 0) {
            return;
        }

        ret = ee_memcheck(page, (uint8_t*) (&mem.phys + addr), len);

        if (ret < 0) {
            mem.write_pending = true;
            mem.dest_loc = addr;
            mem.source_loc = (uint8_t*) (&mem.phys + addr);
            mem.update_len = len;
        }

        addr += MICRO_PG_SIZE;

        if (addr >= sizeof(struct phys_mem)) {
            addr = 0;
            ++search;
        }
    }
    // } else {
    //     ee_memset(page, NAME_LEN, 0);

    //     if (addr == 0) {
    //         addr = mem.phys.pg_bound[search - 1];
    //     }

    //     end = ROUNDUP(mem.phys.pg_bound[search - 1] + mem.pg_size[search - 1], MICRO_PG_SIZE);
    //     len = MICRO_PG_SIZE - (end % MICRO_PG_SIZE);
    //     ret = readMem(addr, page, len);

    //     if (ret < 0) {
    //         return;
    //     }

    //     ret = ee_memcheck(page, (uint8_t*) (&mem.phys + addr - mem.phys.pg_bound[search - 1]), len);

    //     if (ret < 0) {
    //         mem.write_pending = true;
    //         mem.dest_loc = addr;
    //         mem.source_loc = (uint8_t*) (mem.pg_addr[search - 1] + addr - mem.phys.pg_bound[search - 1]);
    //         mem.update_len = len;
    //     }

    //     addr += MICRO_PG_SIZE;

    //     if (addr - mem.phys.pg_bound[search - 1] >= mem.pg_size[search - 1]) {
    //         addr = 0;
    //         ++search;
    //     }
    // }
}

// @funcname: memFg
//
// @brief: Foreground routine for 5ms write coherency
//
// @note: Application code must add to foreground queue
void memFg(void) {
    if (!mem.write_pending) {
        return;
    }

    // TODO: Signal to app code if an error occurs
    writePage(mem.dest_loc, mem.source_loc, mem.update_len);
    mem.write_pending = false;
}

// @funcname: readMem
//
// @brief: Reads memory across micro page boundaries
//
// @param: phys_addr: On chip address of data
// @param: loc_addr: Local address of data
// @param: len: Length of data
//
// @return: E_SUCCESS if read, error code if failed
static int readMem(uint16_t phys_addr, uint8_t* loc_addr, uint16_t len) {
    int      ret;
    uint8_t  page[MICRO_PG_SIZE];
    uint16_t i, size, end;

    // Calculate ending address
    end = ROUNDUP(phys_addr + len, MICRO_PG_SIZE);

    // Read page by page
    for (i = phys_addr; i < end; i += MICRO_PG_SIZE) {
        ret = readPage(i, page);

        if (ret < 0) {
            return ret;
        }

        // Check if micro page would overflow during memcpy
        size = (i + 32 == end) ? MICRO_PG_SIZE - (len % MICRO_PG_SIZE) : MICRO_PG_SIZE;
        ee_memcpy(page, (uint8_t*) (loc_addr + i), size);
    }

    return E_SUCCESS;
}

// @funcname: writePage
//
// @brief: Writes a single micro page to chip
//
// @param: addr: On chip address of data
// @param: page: Pointer to data to write
// @param: size: Length of write (capped at MICRO_PG_SIZE)
//
// @note: E_SUCCESS if written, -E_I2C if failed
static int writePage(uint16_t addr, uint8_t* page, uint8_t size) {
    uint8_t ret;

    size = (size > MICRO_PG_SIZE) ? MICRO_PG_SIZE : size;

    // Pull WC to allow for a write
    PHAL_writeGPIO(mem.wc_gpio_port, mem.wc_gpio_pin, 0);

    // Complete write
    ret += !PHAL_I2C_gen_start(I2C1, MEM_ADDR | MODE_W, size + 2, PHAL_I2C_MODE_TX);
    ret += !PHAL_I2C_write(I2C1, addr >> 8);
    ret += !PHAL_I2C_write(I2C1, addr & 0xff);
    ret += !PHAL_I2C_write_multi(I2C1, page, size);
    ret += !PHAL_I2C_gen_stop(I2C1);

    // Stop mem writes
    PHAL_writeGPIO(mem.wc_gpio_port, mem.wc_gpio_pin, 1);

    return ret ? -E_I2C : E_SUCCESS;
}

// @funcname: readPage
//
// @brief: Reads a single micro page from chip
//
// @param: addr: On chip address of data
// @param: page: Pointer to returned data
//
// @param: E_SUCCESS if read, -E_I2C if failed
static int readPage(uint16_t addr, uint8_t* page) {
    uint8_t ret;

    // Complete read
    ret += !PHAL_I2C_gen_start(I2C1, MEM_ADDR | MODE_W, 2, PHAL_I2C_MODE_TX);
    ret += !PHAL_I2C_write(I2C1, addr >> 8);
    ret += !PHAL_I2C_write(I2C1, addr & 0xff);
    ret += !PHAL_I2C_gen_stop(I2C1);
    ret += !PHAL_I2C_gen_start(I2C1, MEM_ADDR | MODE_R, MICRO_PG_SIZE, PHAL_I2C_MODE_RX);
    ret += !PHAL_I2C_read_multi(I2C1, page, MICRO_PG_SIZE);
    ret += !PHAL_I2C_gen_stop(I2C1);

    return ret ? -E_I2C : E_SUCCESS;
}

// @funcname: memClear
//
// @brief: Clears all addresses to 0 on chip
static void memClear() {
    mem.zero_req = true;
}

// @funcname: memTest
//
// @brief: Fill chip with ones and check each location
//         to ensure writes were complete
//
// @return: E_SUCCESS if memory matches, -E_M_MISMATCH
//
// @note: DO NOT USE! WILL RESET STRUCTS AND CAUSE DEVICE
//        NAKS FOLLOWED BY I2C STOPPAGE
static int memTest() {
    size_t  i;
    uint8_t one[MICRO_PG_SIZE];
    uint8_t page[MICRO_PG_SIZE];

    ee_memset(one, MICRO_PG_SIZE, 0xff);

    for (i = 0; i < CHIP_SIZE; i += MICRO_PG_SIZE) {
        writePage(i, one, MICRO_PG_SIZE);
    }

    for (i = 0; i < CHIP_SIZE; i += MICRO_PG_SIZE) {
        readPage(i, page);

        if (ee_memcheck(page, one, MICRO_PG_SIZE) < 0) {
            return -E_M_MISMATCH;
        }
    }

    return E_SUCCESS;
}

// @funcname: fnameSearch
//
// @brief: Find index
//
// @param: dest: Pointer to location to set
// @param: len: Length to set
// @param: value: Value to set each memory address to
//
// @return: Index of name if it exists, -E_NO_NAME else
static int fnameSearch(char* name) {
    uint8_t i;

    // Search for index of name
    for (i = 0; i < S_FNAME / NAME_LEN; i++) {
        if (ee_memcheck(&mem.phys.filename[i * NAME_LEN], name, NAME_LEN) == 0) {
            return i;
        }
    }

    return -E_NO_NAME;
}

// @funcname: ee_memcheck
//
// @brief: Simple memcheck routine
//
// @param: src: Pointer to location to check
// @param: dest: Pointer to second location to check
// @param: len: Length to check
//
// @return: E_SUCCESS if memory matches, -E_M_MISMATCH else
static int ee_memcheck(uint8_t* src, uint8_t* dest, size_t len) {
    size_t i;

    for (i = 0; i < len; i++) {
        if (src[i] != dest[i]) {
            return -E_M_MISMATCH;
        }
    }

    return E_SUCCESS;
}

// @funcname: ee_memset
//
// @brief: Simple memset routine
//
// @param: dest: Pointer to location to set
// @param: len: Length to set
// @param: value: Value to set each memory address to
static void ee_memset(uint8_t* dest, size_t len, uint8_t value) {
    size_t i;

    for (i = 0; i < len; i++) {
        dest[i] = value;
    }
}

// @funcname: ee_memcpy
//
// @brief: Simple memcpy routine
//
// @param: src: Pointer to location to copy
// @param: dest: Pointer to location from which to copy
// @param: len: Length of data to copy
static void ee_memcpy(uint8_t* src, uint8_t* dest, size_t len) {
    size_t i;

    for (i = 0; i < len; i++) {
        dest[i] = src[i];
    }
}
#endif