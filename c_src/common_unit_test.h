//*****************************************************************************
// Filename: common_unit_test.h
// Author: Fisherman166
//
// Common functions used in all unit tests
//
// ****************************************************************************

#ifndef COMMON_UNIT_TEST_H
#define COMMON_UNIT_TEST_H

#include "common.h"
#include "common_cpu.h"

//*****************************************************************************
// Helper functions
//*****************************************************************************
void my_print(char*);
bool compare_registers(cpu_registers*, uint8_t, uint8_t, uint8_t, uint16_t,
                       uint16_t, uint16_t);

#endif

