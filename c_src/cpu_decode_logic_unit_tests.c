//*****************************************************************************
// Filename: cpu_decode_logic_unit_tests.c
// Author: Fisherman166
//
// Implents all unit tests for cpu decode logic
//
// ****************************************************************************

#include "cpu_decode_logic_unit_tests.h"
#include "cpu_decode_logic.h"
#include "cpu_basic_operations.h"
#include "memory_operations.h"

//*****************************************************************************
// Unit tests
//*****************************************************************************
static void test_immediate() {
    cpu_registers registers;
    init_cpu_registers(&registers, 0, 0, 0, 0x200, 0, 0);

    write_RAM(0x200, 0x15);
    assert(fetch_immediate(&registers) == 0x15);
}


void run_all_cpu_decode_logic_tests() {
    my_print("Running all cpu decode logic tests\n");
    test_immediate();
    my_print("Done testing all cpu decode logic tests\n");
}

