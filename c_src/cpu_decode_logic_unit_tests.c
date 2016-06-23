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


static void test_zeropage() {
    cpu_registers registers;
    init_cpu_registers(&registers, 0, 0, 0, 0x200, 0, 0);

    write_RAM(0x200, 0x15);
    write_RAM(0x15, 0xAC);
    assert(fetch_zeropage(&registers) == 0xAC);

    write_RAM(0x200, 0x87);
    write_RAM(0x87, 0xAD);
    assert(fetch_zeropage(&registers) == 0xAD);
}

static void test_zeropageX() {
    cpu_registers registers;
    init_cpu_registers(&registers, 0, 0x5, 0, 0x200, 0, 0);

    write_RAM(0x200, 0x15);
    write_RAM(0x15, 0xAB);
    write_RAM(0x1A, 0xAC);
    assert(fetch_zeropageX(&registers) == 0xAC);

    write_RAM(0x200, 0xFF);
    write_RAM(0x04, 0xAD);
    assert(fetch_zeropageX(&registers) == 0xAD);
}


void run_all_cpu_decode_logic_tests() {
    my_print("Running all cpu decode logic tests\n");
    test_immediate();
    test_zeropage();
    test_zeropageX();
    my_print("Done testing all cpu decode logic tests\n");
}

