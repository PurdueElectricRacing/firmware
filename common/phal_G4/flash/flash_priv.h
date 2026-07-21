#ifndef PHAL_G4_FLASH_PRIV_H
#define PHAL_G4_FLASH_PRIV_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    bool instruction_cache_enabled;
    bool data_cache_enabled;
} flash_operation_context_t;

bool FLASH_PRIV_begin_read(void);
bool FLASH_PRIV_end_read(void);

bool FLASH_PRIV_begin_operation(flash_operation_context_t *context);
bool FLASH_PRIV_end_operation(const flash_operation_context_t *context);

uint32_t FLASH_PRIV_get_size_bytes(void);
uint32_t FLASH_PRIV_get_page_size_bytes(void);

bool FLASH_PRIV_program_double_word(uint32_t address, uint64_t data);
bool FLASH_PRIV_erase_page(uint32_t page_address);

#endif // PHAL_G4_FLASH_PRIV_H
