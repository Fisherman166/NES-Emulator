set( unit_test_src_files
    common_unit_test.c
    common_unit_test.h
    cpu_basic_operations_unit_tests.c
    cpu_basic_operations_unit_tests.h
    cpu_decode_logic_unit_tests.c
    cpu_decode_logic_unit_tests.h
    cpu_unit_tests.c
    cpu_unit_tests.h
)
add_library(unit_tests ${unit_test_src_files})
