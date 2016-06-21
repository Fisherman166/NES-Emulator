//*****************************************************************************
// Filename: cpu_basic_operations_unit_tests.c
// Author: Fisherman166
//
// Implents all unit tests for cpu basic operations
//
// ****************************************************************************

#include "cpu_basic_operations_unit_tests.h"
#include "cpu_basic_operations.h"

//*****************************************************************************
// Helper function
//*****************************************************************************
static void my_print(char* string) {
    printf("BASIC_CPU_OP_UT: %s", string);
}

static bool compare_registers(cpu_registers* registers, uint8_t A, uint8_t X,
                              uint8_t Y, uint16_t PC, uint16_t S,
                              uint16_t flags)
{
    bool registers_match = 1;
    if(registers->A != A) registers_match = 0;
    else if(registers->X != X) registers_match = 0;
    else if(registers->Y != Y) registers_match = 0;
    else if(registers->PC != PC) registers_match = 0;
    else if(registers->S != S) registers_match = 0;
    else if(registers->flags != flags) registers_match = 0;
    return registers_match;
}

//*****************************************************************************
// Function defintions
//*****************************************************************************
static void test_set_cpu_flag() {
    cpu_registers registers;
    init_cpu_registers(&registers, 0, 0, 0, 0, 0, 0);
    set_cpu_flag(&registers, CARRY_FLAG);
    assert(registers.flags == CARRY_FLAG);
}

static void test_get_cpu_flag() {
    cpu_registers registers;
    init_cpu_registers(&registers, 0, 0, 0, 0, 0, 0);
    get_cpu_flag(&registers, CARRY_FLAG);
    assert(registers.flags == 0);
    set_cpu_flag(&registers, CARRY_FLAG);
    get_cpu_flag(&registers, CARRY_FLAG);
    assert(registers.flags == CARRY_FLAG);
}

static void test_determine_zero_flag() {
    cpu_registers registers;
    init_cpu_registers(&registers, 20, 0, 0, 0, 0, 0);

    determine_zero_flag(&registers, registers.A);
    assert(registers.flags != ZERO_FLAG);
    determine_zero_flag(&registers, 0);
    assert(registers.flags == ZERO_FLAG);
}

static void test_determine_negative_flag() {
    cpu_registers registers;
    init_cpu_registers(&registers, 0, 0, 0, 0, 0, 0);

    determine_negative_flag(&registers, 0);
    assert(registers.flags != NEGATIVE_FLAG);
    determine_negative_flag(&registers, 0x80);
    assert(registers.flags == NEGATIVE_FLAG);
}

static void test_base_add() {
    cpu_registers registers;
    init_cpu_registers(&registers, 0, 0, 0, 0, 0, 0);

    base_add(&registers, 0);
    assert(compare_registers(&registers, 0, 0, 0, 0, 0 , ZERO_FLAG) == 1);
    base_add(&registers, 1);
    assert(compare_registers(&registers, 1, 0, 0, 0, 0, 0) == 1);
    base_add(&registers, 0xFF);
    assert(compare_registers(&registers, 0, 0, 0, 0, 0, CARRY_FLAG | ZERO_FLAG) == 1);
    base_add(&registers, 63); // The carry makes it 64
    assert(compare_registers(&registers, 64, 0, 0, 0, 0, 0) == 1);
    base_add(&registers, 64);
    assert(compare_registers(&registers, 0x80, 0, 0, 0, 0, NEGATIVE_FLAG | OVERFLOW_FLAG) == 1);
}

static void test_base_and() {
    cpu_registers registers;
    init_cpu_registers(&registers, 0xFF, 0, 0, 0, 0, 0);

    base_and(&registers, 0xFF);
    assert(compare_registers(&registers, 0xFF, 0, 0, 0, 0, NEGATIVE_FLAG));
    base_and(&registers, 0xA3);
    assert(compare_registers(&registers, 0xA3, 0, 0, 0, 0, NEGATIVE_FLAG));
    base_and(&registers, 0x03);
    assert(compare_registers(&registers, 0x03, 0, 0, 0, 0, 0));
    base_and(&registers, 0x0);
    assert(compare_registers(&registers, 0, 0, 0, 0, 0, ZERO_FLAG));
}

static void test_base_shift_left() {
    cpu_registers registers;
    init_cpu_registers(&registers, 0x20, 0, 0, 0, 0, 0);

    base_shift_left(&registers, &(registers.A));
    assert(compare_registers(&registers, 0x40, 0, 0, 0, 0, 0) == 1);
    base_shift_left(&registers, &(registers.A));
    assert(compare_registers(&registers, 0x80, 0, 0, 0, 0, NEGATIVE_FLAG) == 1);
    base_shift_left(&registers, &(registers.A));
    assert(compare_registers(&registers, 0x00, 0, 0, 0, 0, CARRY_FLAG | ZERO_FLAG) == 1);
}

static void test_base_bit_test() {
    cpu_registers registers;
    init_cpu_registers(&registers, 0x20, 0, 0, 0, 0, 0);

    base_bit_test(&registers, 0x40);
    assert(compare_registers(&registers, 0x20, 0, 0, 0, 0, ZERO_FLAG | OVERFLOW_FLAG) == 1);
    base_bit_test(&registers, 0xC0);
    assert(compare_registers(&registers, 0x20, 0, 0, 0, 0, ZERO_FLAG | OVERFLOW_FLAG | NEGATIVE_FLAG) == 1);
    base_bit_test(&registers, 0xF0);
    assert(compare_registers(&registers, 0x20, 0, 0, 0, 0, OVERFLOW_FLAG | NEGATIVE_FLAG) == 1);
}

static void test_base_compare() {
    cpu_registers registers;
    init_cpu_registers(&registers, 0x2, 0, 0, 0, 0, 0);

    base_compare(&registers, registers.A, 0x2);
    assert(compare_registers(&registers, 0x2, 0, 0, 0, 0, CARRY_FLAG | ZERO_FLAG) == 1);
    base_compare(&registers, registers.A, 0x1);
    assert(compare_registers(&registers, 0x2, 0, 0, 0, 0, CARRY_FLAG) == 1);
    base_compare(&registers, registers.A, 0x7F);
    assert(compare_registers(&registers, 0x2, 0, 0, 0, 0, NEGATIVE_FLAG) == 1);
}

static void test_base_decrement() {
    cpu_registers registers;
    init_cpu_registers(&registers, 0x2, 0, 0, 0, 0, 0);

    base_decrement(&registers, &(registers.A));
    assert(compare_registers(&registers, 0x1, 0, 0, 0, 0, 0) == 1);
    base_decrement(&registers, &(registers.A));
    assert(compare_registers(&registers, 0x0, 0, 0, 0, 0, ZERO_FLAG) == 1);
    base_decrement(&registers, &(registers.A));
    assert(compare_registers(&registers, 0xFF, 0, 0, 0, 0, NEGATIVE_FLAG) == 1);
}

void run_all_basic_cpu_operations_tests() {
    my_print("Running all basic cpu operation unit tests\n");
    test_set_cpu_flag();
    test_get_cpu_flag();
    test_determine_zero_flag();
    test_determine_negative_flag();
    test_base_add();
    test_base_and();
    test_base_shift_left();
    test_base_bit_test();
    test_base_compare();
    test_base_decrement();
    my_print("Done testing all basic cpu operation unit tests\n");
}

