//*****************************************************************************
// Filename: cpu_basic_operations_unit_tests.c
// Author: Fisherman166
//
// Implents all unit tests for cpu basic operations
//
// ****************************************************************************

#include "cpu_basic_operations_unit_tests.h"
#include "cpu_basic_operations.h"

static void my_print(char* string) {
    printf("BASIC_CPU_OP_UT: %s", string);
}

//*****************************************************************************
// Function defintions
//*****************************************************************************
static void test_set_cpu_flag() {
    cpu_registers registers;
    init_cpu_registers(&registers, 0, 0, 0, 0, 0, 0);
    set_cpu_flag(&registers, CARRY_BIT);
    assert(registers.flags == CARRY_BIT);
}

static void test_get_cpu_flag() {
    cpu_registers registers;
    init_cpu_registers(&registers, 0, 0, 0, 0, 0, 0);
    get_cpu_flag(&registers, CARRY_BIT);
    assert(registers.flags == 0);
    set_cpu_flag(&registers, CARRY_BIT);
    get_cpu_flag(&registers, CARRY_BIT);
    assert(registers.flags == CARRY_BIT);
}

static void test_check_value_for_zero_flag() {
    cpu_registers registers;
    init_cpu_registers(&registers, 20, 0, 0, 0, 0, 0);

    check_value_for_zero_flag(&registers, registers.A);
    assert(registers.flags != ZERO_BIT);
    check_value_for_zero_flag(&registers, 0);
    assert(registers.flags == ZERO_BIT);
}

static void test_check_value_for_negative_flag() {
    cpu_registers registers;
    init_cpu_registers(&registers, 0, 0, 0, 0, 0, 0);

    check_value_for_negative_flag(&registers, 0);
    assert(registers.flags != NEGATIVE_BIT);
    check_value_for_negative_flag(&registers, 0x80);
    assert(registers.flags == NEGATIVE_BIT);
}

void run_all_basic_cpu_operations_tests() {
    my_print("Running all basic cpu operation unit tests\n");
    test_set_cpu_flag();
    test_get_cpu_flag();
    test_check_value_for_zero_flag();
    test_check_value_for_negative_flag();
    my_print("Done testing all basic cpu operation unit tests\n");
}

