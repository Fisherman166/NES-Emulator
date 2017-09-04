set( cpu_src_files
    common_cpu.h
    cpu_basic_operations.c
    cpu_basic_operations.h
    cpu.c
    cpu.h
    cpu_decode_logic.c
    cpu_decode_logic.h
    cpu_instructions.c
    cpu_instructions.h
    illegal_opcodes.h
    illegal_opcodes.c
)
include_directories("./")

add_library(cpu ${cpu_src_files})
