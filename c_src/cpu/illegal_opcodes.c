#include "illegal_opcodes.h"
#include "cpu_basic_operations.h"
#include "cpu_decode_logic.h"

uint8_t zeropage_NOP(cpu_registers* registers) {
    return ZERO_EXTRA_CYCLES;
}

uint8_t zeropageX_NOP(cpu_registers* registers) {
    return ZERO_EXTRA_CYCLES;
}

uint8_t absolute_NOP(cpu_registers* registers) {
    return ZERO_EXTRA_CYCLES;
}

uint8_t absoluteX_NOP(cpu_registers* registers) {
    bool page_crossed;
    fetch_absoluteX(registers, &page_crossed);
    return page_crossed;
}
