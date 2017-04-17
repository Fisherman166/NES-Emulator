set( memory_src_files
    mappers.c
    mappers.h
    memory_operations.c
    memory_operations.h
    VRAM.c
    VRAM.h
    game_loader.c
    game_loader.h
)
add_library(memory ${memory_src_files})
