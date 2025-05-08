#ifndef ____FONTLDR_INC_C__
#define ____FONTLDR_INC_C__

#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>
#include "dazzle/include/dazzle.h"

#ifndef PACKED
    #define PACKED __attribute__((packed))
#endif

//======== File Format Definitions ========//

//PC Scren Font 1
#define PSF1_MAGIC 0x0436
typedef struct {
    uint16_t magic;
    uint8_t mode;
    uint8_t char_size; 
} PACKED psf1_header;

//PC Scren Font 2
#define PSF2_MAGIC 0x864ab572
typedef struct {
    uint32_t magic;
    uint32_t version;
    uint32_t header_size;
    uint32_t flags;
    uint32_t glyph_count;
    uint32_t bytes_per_glyph;
    uint32_t font_height;
    uint32_t font_width;
} PACKED psf2_header;

//Formats enum
typedef enum {
    BT_INVALID_FORMAT,
    BT_FORMAT_PSF1,
    BT_FORMAT_PSF2,
    BT_FORMAT_TTF
} bt_format_t;

//======== Structure Definitions ========//

typedef struct {
    dazzle_allocator_t alloc;
    bt_format_t format;
} font_t;

typedef struct {
    uint32_t* buffer;
    uint32_t width;
    uint32_t height;
} glyph_t; 

// Positioned Glyph Index
typedef struct {
    uint32_t index;
    uint32_t x;
    uint32_t y;
} pgi_t;

//Positioned Glyph Index List
typedef struct {
    pgi_t* pgis;
    uint32_t count;
} pgil_t;

/*
 * @brief Loads a font
 * 
 * @param alloc The allocator to use
 * @param data The font data
 * @param size The size of the font data
 * @param color The color of the font
 * @return font_t The font
*/
font_t load_font(dazzle_allocator_t alloc, char* data, uint32_t size){
    font_t toreturn;

    toreturn.alloc = alloc;

    if(((psf1_header*)data)->magic == PSF1_MAGIC){
        toreturn.format = BT_FORMAT_PSF1;
    }else if(((psf2_header*)data)->magic == PSF2_MAGIC){
        toreturn.format = BT_FORMAT_PSF2;
    } else {
        toreturn.format = BT_INVALID_FORMAT;
    }

    return toreturn;
}
/*
 * @brief Creates a glyph list from text
 * 
 * @param font The font to use
 * @param text The text to create glyphs from
 * @param size The size of the glyphs
 * @return pgil_t The glyph list
*/
pgil_t get_glyphs(font_t font,char* text, uint32_t size){
    
}

/*
 * @brief Renders a glyph
 * 
 * @param font The font to use
 * @param c The character to create a glyph from
 * @param size The size of the glyph
 * @return glyph_t The glyph
*/
glyph_t render_glyph(font_t font, uint32_t c, uint32_t size, uint32_t color){
    return (glyph_t){
        .buffer = NULL,
        .width = 0,
        .height = size
    };
}

#endif // ____FONTLDR_INC_C__