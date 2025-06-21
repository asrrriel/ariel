#include <stdio.h>
#include <string.h>

#define __UTF8_IMPL__
#include "utf8.h"

int main(){
    utf8_dec_state_t state = {0};

    char* ch = "aáeéiíoóuú";

    for(int i = 0; i < strlen(ch); i++){
        uint32_t cp;
        uint8_t status = utf8_decode(&state, ch[i], &cp);
        if(status == UTF8_CODEPOINT_EXTRACTED){
            printf("0x%4x\n", cp);
        } else if (status == UTF8_MORE_BYTES_REQUIRED){
            printf("More bytes required!\n");
        } else if (status == UTF8_INVALID_INPUT){
            printf("Invalid input!\n");
        } else {
            printf("Unknown status!\n");
            return -status;
        }
    }
    return 0;
}