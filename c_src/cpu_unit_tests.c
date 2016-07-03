//*****************************************************************************
// Filename: cpu_unit_tests.c
// Author: Fisherman166
//
// Implents all unit tests for the top level cpu module
//
// ****************************************************************************

#include "cpu_unit_tests.h"
#include "cpu.h"
#include "memory_operations.h"
#include "cpu_basic_operations.h"


//*****************************************************************************
// Function defintions
//*****************************************************************************

static void test_immediate_ADC() {
    write_RAM(0xFFFC, 0xFE);
    write_RAM(0xFFFD, 0x80);
    write_RAM(0x80FE, 0x69);
    write_RAM(0x80FF, 0xDE);

    open_cpu_debug_logfile();
    cpu_registers registers;
    cold_boot_init(&registers);
    execute_interpreter_cycle(&registers);
    assert(compare_registers(&registers, 0xDE, 0, 0, 0x8100, 0xFD, registers.flags | NEGATIVE_FLAG) == 1);

    close_cpu_debug_logfile();
}

void run_all_cpu_tests() {
    my_print("Running all cpu unit tests\n");
    test_immediate_ADC();
    my_print("Done testing all cpu unit tests\n");
}

