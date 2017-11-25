set(src_files
    cpu_basic_operations.c
    cpu.c
    cpu_decode_logic.c
    cpu_instructions.c
    game_loader.c
    illegal_opcodes.c
    main.c
    mappers.c
    ppu.c
    RAM.c
    sdl_interface.c
    sprites.c
    VRAM.c
    controller.c
)
add_library(src ${src_files})
