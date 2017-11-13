#include "common.h"
#include "ppu.h"
#include "RAM.h"
#include "VRAM.h"
#include "sprites.h"

typedef struct {
    bool visable_line;
    bool postrender_line;
    bool prerender_line;
    bool visable_dot;
    bool shift_reload_dot_range;
    bool shift_reload_dot;
    bool next_screen_dot;
    bool render_line;
} line_status;

typedef struct {
    uint16_t    high_background_shift;
    uint16_t    low_background_shift;
    uint16_t    high_attribute_shift;
    uint16_t    low_attribute_shift;
    bool        low_attribute_latch;
    bool        high_attribute_latch;
} BG_shift_registers;

typedef struct {
    uint8_t     high_background_byte;
    uint8_t     low_background_byte;
    uint8_t     nametable_byte;
    uint8_t     attribute_byte;
} fetched_BG_bytes;

typedef struct {
    uint8_t r2000;
    uint8_t r2001;
} ppu_regs;

static const uint32_t RGB_colors[] = {
	//0/8	1/9		2/A	3/B		4/C	5/D		6/E	7/F
   0x00808080, 0x00003DA6, 0x000012B0, 0x00440096, 0x00A1005E, 0x00C70028, 0x00BA0600, 0x008C1700, //0
   0x005C2F00, 0x00104500, 0x00054A00, 0x0000472E, 0x00004166, 0x00000000, 0x00050505, 0x00050505,
   0x00C7C7C7, 0x000077FF, 0x002155FF, 0x008237FA, 0x00EB2FB5, 0x00FF2950, 0x00FF2200, 0x00D63200, //1
   0x00C46200, 0x00358000, 0x00058F00, 0x00008A55, 0x000099CC, 0x00212121, 0x00090909, 0x00090909,
   0x00FFFFFF, 0x000FD7FF, 0x0069A2FF, 0x00D480FF, 0x00FF45F3, 0x00FF618B, 0x00FF8833, 0x00FF9C12, //2
   0x00FABC20, 0x009FE30E, 0x002BF035, 0x000CF0A4, 0x0005FBFF, 0x005E5E5E, 0x000D0D0D, 0x000D0D0D,
   0x00FFFFFF, 0x00A6FCFF, 0x00B3ECFF, 0x00DAABEB, 0x00FFA8F9, 0x00FFABB3, 0x00FFD2B0, 0x00FFEFA6, //3
   0x00FFF79C, 0x00D7E895, 0x00A6EDAF, 0x00A2F2DA, 0x0099FFFC, 0x00DDDDDD, 0x00111111, 0x00111111
};

static uint16_t scanline = 241;
static uint16_t dot = 0;
static bool     odd_frame = false;
static uint32_t pixel_data[SCREEN_HEIGHT][SCREEN_WIDTH];
static bool     NMI_flag = false;


static bool is_visable_line(uint16_t scanline) {
    return scanline <= 239;
}

static bool is_postrender_line(uint16_t scanline) {
    const uint16_t min_scanline = 240;
    const uint16_t max_scanline = 260;
    return (scanline >= min_scanline) && (scanline <= max_scanline);
}

static bool is_prerender_line(uint16_t scanline) {
    const uint16_t prerender_scanline_number = 261;
    return scanline == prerender_scanline_number;
}

static bool is_visable_dot(uint16_t dot) {
    const uint16_t min_dot = 1;
    const uint16_t max_dot = 256;
    return (dot >= min_dot) && (dot <= max_dot);
}

static bool is_shift_reload_dot_range(uint16_t dot) {
    const uint16_t min_dot = 9;
    const uint16_t max_dot = 257;
    return (dot >= min_dot) && (dot <= max_dot);
}

static bool is_next_screen_fetch_dot(uint16_t dot) {
    const uint16_t min_dot = 321;
    const uint16_t max_dot = 336;
    return (dot >= min_dot) && (dot <= max_dot);
}

static bool is_reload_dot(uint16_t dot) {
    // 8 shifts before registers need reloading
    const uint8_t shift_dot_interval = 8;
    return (dot % shift_dot_interval) == 1;
}

static bool is_sprite_rendering_enabled(ppu_regs regs) {
    const uint8_t sprite_enabled_bitmask = 0x10;
    return regs.r2001 & sprite_enabled_bitmask;
}

static bool is_background_rendering_enabled(ppu_regs regs) {
    const uint8_t background_enabled_bitmask = 0x08;
    return regs.r2001 & background_enabled_bitmask;
}

static bool is_rendering_enabled(ppu_regs regs) {
    return is_sprite_rendering_enabled(regs) ||
           is_background_rendering_enabled(regs);
}

static bool should_shift_shift_registers(uint16_t dot) {
    return (dot > 1 && dot < 258) || (dot > 321 && dot < 338);
}

static bool is_secondary_OAM_clear_dot() {
    return (dot >= 1) && (dot <= 64);
}

static bool is_sprite_evuluation_dot() {
    return (dot >= 65) && (dot <= 256);
}

static bool is_sprite_load_dot() {
    return (dot >= 257) && (dot <= 320);
}

static uint8_t get_sprite_size(ppu_regs* ppu_regs) {
    const uint8_t sprite_size_mask = 0x20;
    uint8_t sprite_size = (ppu_regs->r2000 & sprite_size_mask) ? 16 : 8;
    return sprite_size;
}

static uint8_t get_8x8_address(ppu_regs* ppu_regs) {
    const uint8_t sprite_pattern_address_mask = 0x8;
    bool high_addr = ppu_regs->r2000 & sprite_pattern_address_mask;
    uint16_t address = high_addr ? 0x1000 : 0x0;
    return address;
}

static line_status get_ppu_line_status(uint16_t scanline, uint16_t dot) {
    line_status status;
    status.visable_line = is_visable_line(scanline);
    status.postrender_line = is_postrender_line(scanline);
    status.prerender_line = is_prerender_line(scanline);
    status.visable_dot = is_visable_dot(dot);
    status.shift_reload_dot_range = is_shift_reload_dot_range(dot);
    status.shift_reload_dot = is_reload_dot(dot);
    status.next_screen_dot = is_next_screen_fetch_dot(dot);
    status.render_line = status.visable_line || status.prerender_line;
    return status;
}

static ppu_regs get_ppu_registers() {
    ppu_regs regs;
    regs.r2000 = debug_read_RAM(PPUCTRL_ADDRESS);
    regs.r2001 = debug_read_RAM(PPUMASK_ADDRESS);
    return regs;
}

// See scrolling page on NesDev wiki for this info
static uint8_t fetch_nametable_byte(uint16_t VRAM_address) {
    const uint16_t base_nametable_address = 0x2000;
    const uint16_t nametable_bitmask = 0x0FFF;
    uint16_t nametable_address = base_nametable_address |
        (VRAM_address & nametable_bitmask);
    return read_VRAM(nametable_address);
}

// See scrolling page on NesDev wiki for this info
static uint8_t fetch_attribute_byte(uint16_t VRAM_address) {
    const uint16_t base_attribute_address = 0x23C0;
    const uint16_t nametable_select_bitmask = 0x0C00;
    const uint16_t attribute_select_bitmask = 0x38;
    const uint16_t X_Y_select_bitmask = 0x07;
    uint16_t attribute_address = 
        base_attribute_address |
        (VRAM_address & nametable_select_bitmask) |
        ((VRAM_address >> 4) & attribute_select_bitmask) |
        ((VRAM_address >> 2) & X_Y_select_bitmask);
    return read_VRAM(attribute_address);
}

static uint16_t calc_base_tile_address(
    ppu_regs* ppu_registers,
    uint16_t VRAM_address,
    uint8_t nametable_byte
) {
    const uint8_t pattern_table_select_bitmask = 0x10;
    const uint16_t fine_Y_scroll_bitmask = 0x7000;
    uint16_t base_pattern_address;

    if(ppu_registers->r2000 & pattern_table_select_bitmask) 
        base_pattern_address = 0x1000;
    else
        base_pattern_address = 0x0000;
    base_pattern_address |= (nametable_byte << 4) | 
        ((VRAM_address & fine_Y_scroll_bitmask) >> 12);
    return base_pattern_address;
}

static uint8_t fetch_low_pattern_table_byte(
    ppu_regs* ppu_registers,
    uint16_t VRAM_address,
    uint8_t nametable_byte
) {
    uint16_t base_pattern_address = calc_base_tile_address(
        ppu_registers, VRAM_address, nametable_byte);
    return read_VRAM(base_pattern_address);
}

static uint8_t fetch_high_pattern_table_byte(
    ppu_regs* ppu_registers,
    uint16_t VRAM_address,
    uint8_t nametable_byte
) {
    const uint8_t high_tile_offset = 8;
    uint16_t base_pattern_address = calc_base_tile_address(
        ppu_registers, VRAM_address, nametable_byte);
    return read_VRAM(base_pattern_address + high_tile_offset);
}

static bool mux(uint16_t tile, uint8_t bit) {
    return (tile >> bit) & 1;
}

static uint16_t get_pallete_address(BG_shift_registers* BG_regs) {
    const uint16_t pallete_base_address = 0x3F00;
    uint8_t fineX_scroll = get_fineX_scroll();

    return 
        (pallete_base_address |
        mux(BG_regs->low_background_shift, 15 - fineX_scroll) |
        (mux(BG_regs->high_background_shift, 15 - fineX_scroll)) << 1 |
        (mux(BG_regs->low_attribute_shift, 7 - fineX_scroll) << 2) |
        (mux(BG_regs->high_attribute_shift, 7 - fineX_scroll) << 3));
}

void render_pixel(BG_shift_registers* BG_regs, uint16_t scanline, uint16_t dot) {
    uint16_t pallete_address = get_pallete_address(BG_regs);
    uint16_t pallete_data = read_VRAM(pallete_address);
    // Cycle 0 is idle so we have to subtract 1 to get right dot
    pixel_data[scanline][--dot] = RGB_colors[pallete_data];
}

static uint8_t get_attribute_bits_for_tile(
    uint8_t attribute_byte,
    uint16_t VRAM_address
) {
    uint8_t vertical_pos = (VRAM_address >> 4) & 0x4;
    uint8_t horizontal_pos = (VRAM_address - 1) & 0x2;
    uint8_t shift = vertical_pos | horizontal_pos;
    return (attribute_byte >> shift) & 0x3;
}

static void reload_shift_registers(
    BG_shift_registers* BG_regs,
    fetched_BG_bytes* fetched_bytes,
    uint16_t VRAM_address
) {
    BG_regs->low_background_shift |= fetched_bytes->low_background_byte;
    BG_regs->high_background_shift |= fetched_bytes->high_background_byte;
    uint8_t at_bits = get_attribute_bits_for_tile(fetched_bytes->attribute_byte, VRAM_address);
    BG_regs->low_attribute_latch = at_bits & 0x1;
    BG_regs->high_attribute_latch = at_bits & 0x2;
}

static void fetch_shift_register_byte(
    fetched_BG_bytes* fetched_bytes,
    ppu_regs* ppu_regs,
    uint16_t VRAM_address,
    uint16_t dot
) {
    switch( dot % 8 ) {
        case 1:
            fetched_bytes->nametable_byte = fetch_nametable_byte(VRAM_address);
            break;
        case 3:
            fetched_bytes->attribute_byte = fetch_attribute_byte(VRAM_address);
            break;
        case 5:
            fetched_bytes->low_background_byte = fetch_low_pattern_table_byte(ppu_regs, VRAM_address, fetched_bytes->nametable_byte);
            break;
        case 7:
            fetched_bytes->high_background_byte = fetch_high_pattern_table_byte(ppu_regs, VRAM_address, fetched_bytes->nametable_byte);
            break;
    }
}

static void shift_registers(BG_shift_registers* BG_regs) {
    BG_regs->low_background_shift <<= 1;
    BG_regs->high_background_shift <<= 1;
    BG_regs->low_attribute_shift = (BG_regs->low_attribute_shift << 1) |
                                    BG_regs->low_attribute_latch;
    BG_regs->high_attribute_shift = (BG_regs->high_attribute_shift << 1) |
                                     BG_regs->high_attribute_latch;
}

static void execute_ppu(
    ppu_regs* ppu_regs,
    line_status* status,
    uint16_t scanline,
    uint16_t dot
) {
    static BG_shift_registers background_regs = {0, 0, 0, 0, 0, 0};
    static fetched_BG_bytes fetched_bytes = {0, 0, 0, 0};
    uint16_t VRAM_address = get_VRAM_address();

    if(status->visable_line && status->visable_dot)
        render_pixel(&background_regs, scanline, dot);

    if(status->render_line && status->shift_reload_dot_range && status->shift_reload_dot)
        reload_shift_registers(&background_regs, &fetched_bytes, VRAM_address);

    if(status->render_line && status->visable_dot)
        fetch_shift_register_byte(&fetched_bytes, ppu_regs, VRAM_address, dot);

    if(should_shift_shift_registers(dot))
        shift_registers(&background_regs);

    if(status->render_line) {
        if( is_secondary_OAM_clear_dot() )
            secondary_OAM_clear(dot);
        if( is_sprite_evuluation_dot() )
            sprite_evaluation(scanline, dot, get_sprite_size(ppu_regs));
        if( is_sprite_load_dot() )
            do_sprite_load(scanline, dot,
                           get_sprite_size(ppu_regs),
                           get_8x8_address(ppu_regs));
        if(dot == 256)
            incrementY();
        if( (status->visable_dot || status->next_screen_dot) && ((dot % 8) == 0) )
            incrementX();
        if(dot == 257)
            copyX();
        if( status->prerender_line && (dot >= 280) && (dot <= 304) )
            copyY();
        if( status->prerender_line && (dot == 66) )
            clear_sprite0();
    }
}

static void tick(
    ppu_regs regs,
    line_status status,
    uint16_t* scanline,
    uint16_t* dot
) {
    const uint16_t max_dot = 341;

    // When BG rendering is enabled and we are on an odd frame, skip from 261, 339 -> 0, 0
    if( is_background_rendering_enabled(regs) ) {
        if( odd_frame && status.prerender_line && (*dot == 339) ) {
            *scanline = 0;
            *dot = 0;
            odd_frame ^= 1;
            return;
        }
    }

    *dot += 1;
    if(*dot >= max_dot) {
        *dot -= max_dot;
        if(status.prerender_line) {
            *scanline = 0;
            odd_frame ^= 1;
        }
        else *scanline += 1;
    }
}

static bool is_entering_vblank(uint16_t scanline, uint16_t dot) {
    return (scanline == 241) && (dot == 1);
}

static bool is_exiting_vblank(uint16_t scanline, uint16_t dot) {
    return (scanline == 261) && (dot == 1);
}

static bool check_VBlank(
    ppu_regs regs, 
    uint16_t scanline, 
    uint16_t dot, 
    bool* NMI_flag
) {
    static bool vblank = false;

    if(is_entering_vblank(scanline, dot)) {
        set_vblank_bit();
        vblank = true;
        if(is_NMI_set_in_RAM()) *NMI_flag = 1;
    }
    else if(is_exiting_vblank(scanline, dot)) {
        clear_vblank_bit();
        vblank = false;
    }
    return vblank;
}

//*****************************************************************************
// Public functions
//*****************************************************************************
bool run_PPU_cycle() {
    line_status status = get_ppu_line_status(scanline, dot);
    ppu_regs ppu_regs = get_ppu_registers();

    if(is_rendering_enabled(ppu_regs))
        execute_ppu(&ppu_regs, &status, scanline, dot);

    tick(ppu_regs, status, &scanline, &dot);
    return check_VBlank(ppu_regs, scanline, dot, &NMI_flag);
}

uint32_t* get_pixel_data_ptr() {
    return &pixel_data[0][0];
}

uint16_t get_scanline() { return scanline; }
uint16_t get_dot() { return dot; }

bool get_NMI_flag() { return NMI_flag; }
void clear_nmi_flag() { NMI_flag = 0; }

