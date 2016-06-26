//*****************************************************************************
// Filename: cpu_operations.c
// Author: Fisherman166
//
// Implements the actual cpu_operations
//
//*****************************************************************************

#include "cpu_operations.h"
#include "cpu_basic_operations.h"
#include "cpu_decode_logic.h"

void immediate_ADC(cpu_registers* registers) {
    uint8_t data = fetch_immediate(registers);
    base_add(registers, data);
}


