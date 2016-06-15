//*****************************************************************************
// Filename: cpu_basic_operations_unit_tests.c
// Author: Fisherman166
//
// Implents all unit tests for cpu basic operations
//
// ****************************************************************************

#include "cpu_basic_operations_unit_tests.h"
#include "cpu_basic_operations.h"


static void my_print(char* string);

//*****************************************************************************
// Function defintions
//*****************************************************************************
static void test_set_cpu_flag() {
    cpu_registers registers;
    init_cpu_registers(&registers, 0, 0, 0, 0, 0, 0);

    my_print("Running all set cpu flag tests\n");
    set_cpu_flag(&registers, CARRY_BIT);
    assert(registers.flags == CARRY_BIT);

    my_print("Done with all set cpu flag tests\n");
}

static void test_get_cpu_flag() {
    cpu_registers registers;
    init_cpu_registers(&registers, 0, 0, 0, 0, 0, 0);

    my_print("Running all get_cpu_flag tests\n");
    get_cpu_flag(&registers, CARRY_BIT);
    assert(registers.flags == 0);
    set_cpu_flag(&registers, CARRY_BIT);
    get_cpu_flag(&registers, CARRY_BIT);
    assert(registers.flags == CARRY_BIT);

    my_print("Done with all get_cpu_flag tests\n");
}

static void my_print(char* string) {
    printf("BASIC_CPU_OP_UT: %s", string);
}

void run_all_basic_cpu_operations_tests() {
    my_print("Running all basic cpu operation unit tests\n");
    test_set_cpu_flag();
    test_get_cpu_flag();
    my_print("Done testing all basic cpu operation unit tests\n");
}

