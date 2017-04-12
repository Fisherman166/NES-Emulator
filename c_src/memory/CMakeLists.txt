set( memory_src_files
    mappers.c
    mappers.h
    memory_operations.c
    memory_operations.h
    VRAM.c
    VRAM.h
)
add_library(memory ${memory_src_files})
