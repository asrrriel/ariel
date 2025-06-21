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

#ifdef __UTF8_IMPL__
    uint8_t utf8_decode(utf8_dec_state_t *state, uint8_t c, uint32_t *out){
        if (state->bytes_remaining == 0){
            if (c == 0xFF) {
                return UTF8_INVALID_INPUT;
            } else if (c >= 0xf0){ // 4 bytes long
                state->num_bytes = 4;
                state->bytes_remaining = 3;
                state->codepoint = c & 0x07;
                return UTF8_MORE_BYTES_REQUIRED;
            } else if (c >= 0xe0){ // 3 bytes long
                state->num_bytes = 3;
                state->bytes_remaining = 2;
                state->codepoint = c & 0x0F;
                return UTF8_MORE_BYTES_REQUIRED;
            } else if (c >= 0xc0){ // 2 bytes long
                state->num_bytes = 2;
                state->bytes_remaining = 1;
                state->codepoint = c & 0x1F;
                return UTF8_MORE_BYTES_REQUIRED;
            } else if (c >= 0x80){ // continuation byte, not supposed to be here
                return UTF8_INVALID_INPUT;
            } else { // 1 byte long(may be a non-ascii invalid character)
                *out = (uint32_t)c;
                return UTF8_CODEPOINT_EXTRACTED;
            }
        } else {
            if(!(c & 0x80)){
                return UTF8_INVALID_INPUT;
            } 
            state->codepoint = (c & 0x3F) | (state->codepoint << 6);
            state->bytes_remaining--;
            if(state->bytes_remaining == 0){
                *out = state->codepoint;
                return UTF8_CODEPOINT_EXTRACTED;
            }
            return UTF8_MORE_BYTES_REQUIRED;
        }
    }
#endif

#endif // __UTF8_H__