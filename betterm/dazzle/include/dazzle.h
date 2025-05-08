/*
 * dazzle.h
 * This is the core header file for Dazzle
 * 2025-03-18
 * 
 * This file is part of the public domain RoidsOS operating system.
 *
 * Licensed under CC0, see https://creativecommons.org/publicdomain/zero/1.0/
 *
 * NO WARRANTY EXPRESSED OR IMPLIED. USE AT YOUR OWN RISK. 
 */
#ifndef __DAZZLE_H__
#define __DAZZLE_H__
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

//======== Basic Information ========//
#define DAZZLE_VERSION_MAJOR 0
#define DAZZLE_VERSION_MINOR 0
#define DAZZLE_VERSION_PATCH 1

//======== Defines ========//
#define DAZZLE_RENDERER_TYPE_FRAMEBUFFER 0

#define DAZZLE_RETAINED_TRIANGLE 0
#define DAZZLE_RETAINED_RECTANGLE 1
#define DAZZLE_RETAINED_QUAD 2
#define DAZZLE_RETAINED_CIRCLE 3
#define DAZZLE_RETAINED_BLITABLE 4


//======== Structure Definitions ========//
typedef struct {
    void* (*malloc)(size_t size);
    void (*free)(void* ptr);
} dazzle_allocator_t;

typedef struct retained {
    uint8_t type;
    union {
        struct {
            uint32_t x1;
            uint32_t y1;
            uint32_t x2;
            uint32_t y2;
            uint32_t x3;
            uint32_t y3;
            bool filled;
            uint64_t color;
        } triangle;
        struct {
            uint32_t x1;
            uint32_t y1;
            uint32_t x2;
            uint32_t y2;
            uint32_t x3;
            uint32_t y3;
            uint32_t x4;
            uint32_t y4;
            bool filled;
            uint64_t color;
        } quad;
        struct {
            uint32_t x;
            uint32_t y;
            uint32_t width;
            uint32_t height;
            bool filled;
            uint64_t color;
        } rect;
        struct {
            uint32_t x;
            uint32_t y;
            uint32_t radius;
            bool filled;
            uint64_t color;
        } circle;
        struct {
            uint32_t x;
            uint32_t y;
            uint32_t width;
            uint32_t height;
            bool translated;
            void* buffer;
        } blit;
    } type_data;
    struct retained* next;
} dazzle_retained_element_t;

typedef struct dazzle_context_t {
    //Required stuff//
    dazzle_allocator_t alloc;

    //Renderer functions//
    bool (*draw_element)(struct dazzle_context_t* ctx, dazzle_retained_element_t* element);
    bool (*clear)(struct dazzle_context_t* ctx, uint64_t color);

    //Retained state//
    uint32_t retained_count;
    dazzle_retained_element_t* retained;
    dazzle_retained_element_t* last;

    //Renderer data//
    void* renderer_data;
} dazzle_context_t;

//======== Function Prototypes ========//
//Immediate rendering//
bool dazzle_clear(dazzle_context_t* ctx, uint64_t color);

//Element creation//
dazzle_retained_element_t* dazzle_create_triangle(dazzle_context_t* ctx, uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t x3, uint32_t y3,bool filled,uint64_t color);
dazzle_retained_element_t* dazzle_create_rectangle(dazzle_context_t* ctx, uint32_t x, uint32_t y, uint32_t width, uint32_t height,bool filled,uint64_t color);
dazzle_retained_element_t* dazzle_create_circle(dazzle_context_t* ctx, uint32_t x, uint32_t y, uint32_t radius,bool filled,uint64_t color);
dazzle_retained_element_t* dazzle_create_blitable(dazzle_context_t* ctx, uint32_t x, uint32_t y, uint32_t width, uint32_t height, void* buffer);

//Retained rendering//
bool dazzle_redraw(dazzle_context_t* ctx);
bool dazzle_add(dazzle_context_t* ctx, dazzle_retained_element_t* element);

//======== Function Implementations ========//
#ifdef __DAZZLE_IMPL__

bool dazzle_clear(dazzle_context_t* ctx, uint64_t color){
    return ctx->clear(ctx,color);
}

dazzle_retained_element_t* dazzle_create_triangle(dazzle_context_t* ctx, uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t x3, uint32_t y3,bool filled,uint64_t color){
    dazzle_retained_element_t* e = ctx->alloc.malloc(sizeof(dazzle_retained_element_t));

    if(e == NULL) return false;

    e->type = DAZZLE_RETAINED_TRIANGLE;
    e->type_data.triangle.x1 = x1;
    e->type_data.triangle.y1 = y1;
    e->type_data.triangle.x2 = x2;
    e->type_data.triangle.y2 = y2;
    e->type_data.triangle.x3 = x3;
    e->type_data.triangle.y3 = y3;
    e->type_data.triangle.filled = filled;
    e->type_data.triangle.color = color;
    e->next = NULL;

    return e;
}

dazzle_retained_element_t* dazzle_create_rectangle(dazzle_context_t* ctx, uint32_t x, uint32_t y, uint32_t width, uint32_t height,bool filled,uint64_t color){
    dazzle_retained_element_t* e = ctx->alloc.malloc(sizeof(dazzle_retained_element_t));

    if(e == NULL) return false;

    e->type = DAZZLE_RETAINED_RECTANGLE;
    e->type_data.rect.x = x;
    e->type_data.rect.y = y;
    e->type_data.rect.width = width;
    e->type_data.rect.height = height;
    e->type_data.rect.filled = filled;
    e->type_data.rect.color = color;
    e->next = NULL;

    return e;
}

dazzle_retained_element_t* dazzle_create_circle(dazzle_context_t* ctx, uint32_t x, uint32_t y, uint32_t radius,bool filled,uint64_t color){
    dazzle_retained_element_t* e = ctx->alloc.malloc(sizeof(dazzle_retained_element_t));

    if(e == NULL) return false;

    e->type = DAZZLE_RETAINED_CIRCLE;
    e->type_data.circle.x = x;
    e->type_data.circle.y = y;
    e->type_data.circle.radius = radius;
    e->type_data.circle.filled = filled;
    e->type_data.circle.color = color;
    e->next = NULL;

    return e;
}

dazzle_retained_element_t* dazzle_create_blitable(dazzle_context_t* ctx, uint32_t x, uint32_t y, uint32_t width, uint32_t height, void* buffer){
    dazzle_retained_element_t* e = ctx->alloc.malloc(sizeof(dazzle_retained_element_t));

    if(e == NULL) return false;

    e->type = DAZZLE_RETAINED_BLITABLE;
    e->type_data.blit.x = x;
    e->type_data.blit.y = y;
    e->type_data.blit.width = width;
    e->type_data.blit.height = height;
    e->type_data.blit.buffer = buffer;
    e->next = NULL;

    return e;
}

bool dazzle_redraw(dazzle_context_t* ctx){
    dazzle_retained_element_t* e = ctx->retained;
    bool success = true;
    while(e != NULL){
        success &= ctx->draw_element(ctx,e);
        e = e->next;
    }
    return success;
}
bool dazzle_add(dazzle_context_t* ctx, dazzle_retained_element_t* e){
    if(ctx->retained == NULL){
        ctx->retained = e;
        ctx->last = e;
    } else {
        ctx->last->next = e;
        ctx->last = e;
    }
    ctx->retained_count++;
}

#endif

//======== Backend inclusions ========//
#include "backends/framebuf.inc.c"

#endif // __DAZZLE_H__