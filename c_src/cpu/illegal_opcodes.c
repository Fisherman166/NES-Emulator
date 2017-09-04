#include "illegal_opcodes.h"
#include "cpu_basic_operations.h"
#include "cpu_decode_logic.h"

uint8_t immediate_NOP(cpu_registers* registers) {return ZERO_EXTRA_CYCLES;}
uint8_t zeropage_NOP(cpu_registers* registers) {return ZERO_EXTRA_CYCLES;}
uint8_t zeropageX_NOP(cpu_registers* registers) {return ZERO_EXTRA_CYCLES;}
uint8_t absolute_NOP(cpu_registers* registers) {return ZERO_EXTRA_CYCLES;}

uint8_t absoluteX_NOP(cpu_registers* registers) {
    bool page_crossed;
    fetch_absoluteX(registers, &page_crossed);
    return page_crossed;
}

uint8_t zeropage_LAX(cpu_registers* registers) {
    uint8_t data = fetch_zeropage(registers);
    base_load_register(registers, &(registers->X), data);
    base_load_register(registers, &(registers->A), data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t zeropageY_LAX(cpu_registers* registers) {
    uint8_t data = fetch_zeropageY(registers);
    base_load_register(registers, &(registers->X), data);
    base_load_register(registers, &(registers->A), data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t absolute_LAX(cpu_registers* registers) {
    uint8_t data = fetch_absolute(registers);
    base_load_register(registers, &(registers->X), data);
    base_load_register(registers, &(registers->A), data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t absoluteY_LAX(cpu_registers* registers) {
    bool page_crossed;
    uint8_t data = fetch_absoluteY(registers, &page_crossed);
    base_load_register(registers, &(registers->X), data);
    base_load_register(registers, &(registers->A), data);
    return page_crossed;
}

uint8_t indirectX_LAX(cpu_registers* registers) {
    uint8_t data = fetch_indirectX(registers);
    base_load_register(registers, &(registers->X), data);
    base_load_register(registers, &(registers->A), data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t indirectY_LAX(cpu_registers* registers) {
    bool page_crossed;
    uint8_t data = fetch_indirectY(registers, &page_crossed);
    base_load_register(registers, &(registers->X), data);
    base_load_register(registers, &(registers->A), data);
    return page_crossed;
}

uint8_t zeropage_SAX(cpu_registers* registers) {
    uint8_t data = registers->A & registers->X;
    base_store(calc_zeropage_address(registers), &data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t zeropageY_SAX(cpu_registers* registers) {
    uint8_t data = registers->A & registers->X;
    base_store(calc_zeropageY_address(registers), &data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t absolute_SAX(cpu_registers* registers) {
    uint8_t data = registers->A & registers->X;
    base_store(calc_absolute_address(registers), &data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t indirectX_SAX(cpu_registers* registers) {
    uint8_t data = registers->A & registers->X;
    base_store(calc_indirectX_address(registers), &data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t zeropage_DCP(cpu_registers* registers) {
    uint8_t data = fetch_zeropage(registers) - 1;
    base_store(calc_zeropage_address(registers), &data);
    base_compare(registers, registers->A, data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t zeropageX_DCP(cpu_registers* registers) {
    uint8_t data = fetch_zeropageX(registers) - 1;
    base_store(calc_zeropageX_address(registers), &data);
    base_compare(registers, registers->A, data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t absolute_DCP(cpu_registers* registers) {
    uint8_t data = fetch_absolute(registers) - 1;
    base_store(calc_absolute_address(registers), &data);
    base_compare(registers, registers->A, data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t absoluteX_DCP(cpu_registers* registers) {
    uint8_t data = fetch_absoluteX(registers, NULL) - 1;
    base_store(calc_absoluteX_address(registers), &data);
    base_compare(registers, registers->A, data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t absoluteY_DCP(cpu_registers* registers) {
    uint8_t data = fetch_absoluteY(registers, NULL) - 1;
    base_store(calc_absoluteY_address(registers), &data);
    base_compare(registers, registers->A, data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t indirectX_DCP(cpu_registers* registers) {
    uint8_t data = fetch_indirectX(registers) - 1;
    base_store(calc_indirectX_address(registers), &data);
    base_compare(registers, registers->A, data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t indirectY_DCP(cpu_registers* registers) {
    uint8_t data = fetch_indirectY(registers, NULL) - 1;
    base_store(calc_indirectY_address(registers), &data);
    base_compare(registers, registers->A, data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t zeropage_ISB(cpu_registers* registers) {
    uint8_t data = fetch_zeropage(registers) + 1;
    base_store(calc_zeropage_address(registers), &data);
    base_subtract(registers, data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t zeropageX_ISB(cpu_registers* registers) {
    uint8_t data = fetch_zeropageX(registers) + 1;
    base_store(calc_zeropageX_address(registers), &data);
    base_subtract(registers, data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t absolute_ISB(cpu_registers* registers) {
    uint8_t data = fetch_absolute(registers) + 1;
    base_store(calc_absolute_address(registers), &data);
    base_subtract(registers, data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t absoluteX_ISB(cpu_registers* registers) {
    uint8_t data = fetch_absoluteX(registers, NULL) + 1;
    base_store(calc_absoluteX_address(registers), &data);
    base_subtract(registers, data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t absoluteY_ISB(cpu_registers* registers) {
    uint8_t data = fetch_absoluteY(registers, NULL) + 1;
    base_store(calc_absoluteY_address(registers), &data);
    base_subtract(registers, data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t indirectX_ISB(cpu_registers* registers) {
    uint8_t data = fetch_indirectX(registers) + 1;
    base_store(calc_indirectX_address(registers), &data);
    base_subtract(registers, data);
    return ZERO_EXTRA_CYCLES;
}

uint8_t indirectY_ISB(cpu_registers* registers) {
    uint8_t data = fetch_indirectY(registers, NULL) + 1;
    base_store(calc_indirectY_address(registers), &data);
    base_subtract(registers, data);
    return ZERO_EXTRA_CYCLES;
}
