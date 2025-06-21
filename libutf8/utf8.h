#ifndef __UTF8_H__
#define __UTF8_H__

#include <stdint.h>
#include <sys/types.h>

#define UTF8_CODEPOINT_EXTRACTED 0
#define UTF8_MORE_BYTES_REQUIRED 1
#define UTF8_INVALID_INPUT       2

typedef struct {
    uint8_t bytes_remaining;
    uint8_t num_bytes;
    uint32_t codepoint;
} utf8_dec_state_t;

#ifdef __UTF8_IMPL__
    #define __UTF8_INTERNAL_BYTEPAT_CONT 0b10000000 
    #define __UTF8_INTERNAL_BYTEPAT_2    0b11000000
    #define __UTF8_INTERNAL_BYTEPAT_3    0b11100000
    #define __UTF8_INTERNAL_BYTEPAT_4    0b11110000
    /*
     * @brief Decodes a utf8 character
     * 
     * @param state The state of the decoder
     * @param c The byte to decode
     * @param out The decoded character
     * 
     * @return The status code
    */
    uint8_t utf8_decode(utf8_dec_state_t *state, uint8_t c, uint32_t *out){
        if (state->bytes_remaining == 0){
            if (c & __UTF8_INTERNAL_BYTEPAT_CONT){
                if (c >= __UTF8_INTERNAL_BYTEPAT_2){
                    state->num_bytes = 2;
                    state->bytes_remaining = 1;
                    state->codepoint = c & ~__UTF8_INTERNAL_BYTEPAT_2;
                    return UTF8_MORE_BYTES_REQUIRED;
                } else if (c >= __UTF8_INTERNAL_BYTEPAT_3){
                    state->num_bytes = 3;
                    state->bytes_remaining = 2;
                    state->codepoint = c & ~__UTF8_INTERNAL_BYTEPAT_3;
                    return UTF8_MORE_BYTES_REQUIRED;
                } else if (c >= __UTF8_INTERNAL_BYTEPAT_4){
                    state->num_bytes = 4;
                    state->bytes_remaining = 3;
                    state->codepoint = c & ~__UTF8_INTERNAL_BYTEPAT_4;
                    return UTF8_MORE_BYTES_REQUIRED;
                }
            } else {
                *out = (uint32_t)c;
                return UTF8_CODEPOINT_EXTRACTED;
            }
        } else {
            if(!(c & __UTF8_INTERNAL_BYTEPAT_CONT)){
                return UTF8_INVALID_INPUT;
            } 
    
            state->codepoint = (c & ~__UTF8_INTERNAL_BYTEPAT_2) | (state->codepoint << 6);
            state->bytes_remaining--;

            if(state->bytes_remaining == 0){
                *out = state->codepoint;
                return UTF8_CODEPOINT_EXTRACTED;
            }
            return UTF8_MORE_BYTES_REQUIRED;
        }
    }
#else
    /*
     * @brief Decodes a utf8 character
     * 
     * @param state The state of the decoder
     * @param c The byte to decode
     * @param out The decoded character
     * 
     * @return The status code
    */
    uint8_t utf8_decode(utf8_dec_state_t *state, uint8_t c, uint32_t *out);
#endif

#endif // __UTF8_H__