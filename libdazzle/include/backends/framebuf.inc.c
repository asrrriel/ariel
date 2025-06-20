 #ifndef __FRAMEBUF_INC_C__
 #define __FRAMEBUF_INC_C__
 
 //======== Structure Definitions ========//
 typedef struct
 {
     // Basic info//
     uintptr_t address;
     uint32_t width;
     uint32_t height;
     uint32_t pitch;
 
     // Pixel format//
     uint32_t bpp;
     uint8_t red_mask;
     uint8_t green_mask;
     uint8_t blue_mask;
     uint8_t alpha_mask;
     uint8_t red_shift;
     uint8_t green_shift;
     uint8_t blue_shift;
     uint8_t alpha_shift;
 } dazzle_framebuffer_t;
 
 //======== Macro Definitions ========//

 #define SWAP(a, b)          \
     do                      \
     {                       \
         typeof(a) temp = a; \
         a = b;              \
         b = temp;           \
     } while (0)
 

 //======== Function Prototypes ========//
 
 /*
  * dazzle_init_fb(alloc,fb) -> dazzle_context_t*
  * Initializes a new Dazzle framebuffer renderer using the framebuffer fb
  */
 dazzle_context_t *dazzle_init_fb(dazzle_allocator_t alloc, dazzle_framebuffer_t *fb);
 

 //======== Function Implementations ========//
 #ifdef __DAZZLE_IMPL__
 
 uint64_t __convert_color(dazzle_framebuffer_t *fb, uint64_t color)
 {
     uint64_t converted = 0;
 
     converted |= ((color & 0xFF) & fb->red_mask) << fb->red_shift;
     converted |= (((color & 0xFF00) >> 8) & fb->green_mask) << fb->green_shift;
     converted |= (((color & 0xFF0000) >> 16) & fb->blue_mask) << fb->blue_shift;
     converted |= (((color & 0xFF000000) >> 24) & fb->alpha_mask) << fb->alpha_shift;
 
     return converted;
 }

 void draw_span(dazzle_framebuffer_t *fb, uint32_t x, uint32_t y, uint32_t width, void* linebuf)
 {
     if (fb == NULL || linebuf == NULL || x >= fb->width || y >= fb->height) // check for invalid stuff
         return; 

    uint32_t bypp = fb->bpp / 8;

    memcpy((void *)fb->address + (y * fb->pitch) + (x * bypp), linebuf, width * bypp);
 }
 
 bool dazzle_fb_clear(dazzle_context_t *ctx, uint64_t color)
 {
    if (ctx->renderer_data == NULL)
        return false;
    dazzle_framebuffer_t *fb = (dazzle_framebuffer_t *)ctx->renderer_data;

    uint64_t converted = __convert_color(fb, color);

    uint32_t bypp = fb->bpp / 8;

    void *linebuf = ctx->alloc.malloc(fb->pitch * 2);
    for (int j = 0; j < fb->width * 2; j++)
    {
        memcpy(linebuf + (j * bypp), &converted, bypp);
    }

    for(int i = 0; i < fb->height / 2; i++){
        draw_span(fb, 0, i * 2, fb->width * 2, linebuf);
    }
    ctx->alloc.free(linebuf);
    return true;
 }
 
 bool dazzle_fb_draw_element(dazzle_context_t *ctx, dazzle_retained_element_t *e)
 {
     if (ctx->renderer_data == NULL)
         return false;
     dazzle_framebuffer_t *fb = (dazzle_framebuffer_t *)ctx->renderer_data;
     uint64_t color = 0;
     uint32_t bypp = fb->bpp / 8;
     void* linebuf = NULL;
 
     switch (e->type)
     {
     case DAZZLE_RETAINED_TRIANGLE:
         color = __convert_color(fb, e->type_data.triangle.color);
         break;
     case DAZZLE_RETAINED_RECTANGLE:
         color = __convert_color(fb, e->type_data.rect.color);
         break;
     case DAZZLE_RETAINED_CIRCLE:
         color = __convert_color(fb, e->type_data.circle.color);
         break;
     case DAZZLE_RETAINED_BLITABLE:
        if(!e->type_data.blit.translated){
           uint32_t* newbuf = ctx->alloc.malloc(e->type_data.blit.width * e->type_data.blit.height * bypp);
           uint32_t size = e->type_data.blit.width * e->type_data.blit.height;
           for(uint32_t i = 0; i < size; i++){
                uint32_t col = ((uint32_t*)e->type_data.blit.buffer)[i];
                newbuf[i] = (uint32_t)__convert_color(fb, col);
           }
           e->type_data.blit.buffer = newbuf;
           e->type_data.blit.translated = true;
        }
     }

    linebuf = ctx->alloc.malloc(fb->pitch);
    if(linebuf == NULL)
        return false;

    for (int j = 0; j < fb->width; j++)
    {
        memcpy(linebuf + (j * bypp), &color, bypp);
    }
 
     if (ctx->renderer_data == NULL)
         return false;
     switch (e->type)
     {
        case DAZZLE_RETAINED_RECTANGLE:
            uint32_t top = e->type_data.rect.y;
            uint32_t bottom = e->type_data.rect.y + e->type_data.rect.height - 1;
            uint32_t left = e->type_data.rect.x;
            uint32_t right = e->type_data.rect.x + e->type_data.rect.width - 1;
            if (e->type_data.rect.filled)
            {
                for (int i = top; i < bottom; i++)
                {
                    draw_span(fb, left, i, e->type_data.rect.width, linebuf);
                }
            }
            else
            {
                //Top and bottom
                draw_span(fb, left, top,    e->type_data.rect.width, linebuf);
                draw_span(fb, left, bottom, e->type_data.rect.width, linebuf);

                //Left and right
                for (int i = top + 1; i < bottom; i++)
                {
                    draw_span(fb, left,  i, 1, linebuf);
                    draw_span(fb, right, i, 1, linebuf);
                }
            }
            break;
        case DAZZLE_RETAINED_BLITABLE:
            for (int i = 0; i < e->type_data.blit.height; i++)
            {
                memcpy((uint8_t *)fb->address + ((e->type_data.blit.y + i) * fb->pitch) + (e->type_data.blit.x * bypp), e->type_data.blit.buffer + (i * e->type_data.blit.width * bypp), e->type_data.blit.width * bypp);
            }
            break;
        case DAZZLE_RETAINED_TRIANGLE:
            uint32_t x1 = e->type_data.triangle.x1, y1 = e->type_data.triangle.y1;
            uint32_t x2 = e->type_data.triangle.x2, y2 = e->type_data.triangle.y2;
            uint32_t x3 = e->type_data.triangle.x3, y3 = e->type_data.triangle.y3;

            if (y1 == y3) return false;

            // Sort vertices by Y coordinate
            if (y1 > y2)
            {
                SWAP(x1, x2);
                SWAP(y1, y2);
            }
            if (y1 > y3)
            {
                SWAP(x1, x3);
                SWAP(y1, y3);
            }
            if (y2 > y3)
            {
                SWAP(x2, x3);
                SWAP(y2, y3);
            }

            // Compute slopes (fixed-point math)
            int dx1 = (y2 != y1) ? ((x2 - x1) * 0xffff) / (y2 - y1) : 0;
            int dx2 = (y3 != y1) ? ((x3 - x1) * 0xffff) / (y3 - y1) : 0;
            int dx3 = (y3 != y2) ? ((x3 - x2) * 0xffff) / (y3 - y2) : 0;

            int xL = x1 * 0xffff, xR = (x1 + 1) * 0xffff;

            // Draw upper part of the triangle
            for (uint32_t i = y1; i < y2; i++)
            {
                int x_start = xL / 0xffff;
                int x_end = xR / 0xffff;

                if (x_start > x_end)
                    SWAP(x_start, x_end);
                if (x_start < 0)
                    x_start = 0;
                if (x_end >= fb->width)
                    x_end = fb->width - 1;

                draw_span(fb, x_start, i, x_end - x_start, linebuf);
                xL += dx1;
                xR += dx2;
            }

            // Draw lower part of the triangle
            xL = x2 * 0xffff;
            for (uint32_t i = y2; i < y3; i++)
            {
                int x_start = xL / 0xffff;
                int x_end = xR / 0xffff;

                if (x_start > x_end)
                    SWAP(x_start, x_end);
                if (x_start < 0)
                    x_start = 0;
                if (x_end >= fb->width)
                    x_end = fb->width - 1;

                draw_span(fb, x_start, i, x_end - x_start, linebuf);
                xL += dx3;
                xR += dx2;
            }
            break;
        case DAZZLE_RETAINED_CIRCLE:
            int cx = e->type_data.circle.x, cy = e->type_data.circle.y, r = e->type_data.circle.radius;
            int x = r, y = 0;
            int p = 1 - r; // Initial decision parameter
            while (x >= y)
            {
                // Draw symmetrical points
                if (e->type_data.circle.filled)
                {
                    draw_span(fb, cx - x, cy + y, 2 * x + 1, linebuf);
                    draw_span(fb, cx - x, cy - y, 2 * x + 1, linebuf);
                    draw_span(fb, cx - y, cy + x, 2 * y + 1, linebuf);
                    draw_span(fb, cx - y, cy - x, 2 * y + 1, linebuf);
                }
                else {
                    draw_span(fb, cx + x, cy + y, 1, linebuf);
                    draw_span(fb, cx - x, cy + y, 1, linebuf);
                    draw_span(fb, cx + x, cy - y, 1, linebuf);
                    draw_span(fb, cx - x, cy - y, 1, linebuf);
                    draw_span(fb, cx + y, cy + x, 1, linebuf);
                    draw_span(fb, cx - y, cy + x, 1, linebuf);
                    draw_span(fb, cx + y, cy - x, 1, linebuf);
                    draw_span(fb, cx - y, cy - x, 1, linebuf);
                }

                y++; // Move to next scanline
                if (p <= 0)
                {
                    p += 2 * y + 1;
                }
                else
                {
                    x--;
                    p += 2 * (y - x) + 1;
                }
            }
            break;
     }
     if (linebuf != NULL)
         ctx->alloc.free(linebuf);
     return true;
 }
 
 dazzle_context_t *dazzle_init_fb(dazzle_allocator_t alloc, dazzle_framebuffer_t *fb)
 {
     dazzle_context_t *ctx = alloc.malloc(sizeof(dazzle_context_t));
 
     if (ctx == NULL)
         return NULL;
 
     ctx->alloc = alloc;
     ctx->retained_count = 0;
     ctx->retained = NULL;
     ctx->last = NULL;
     ctx->renderer_data = alloc.malloc(sizeof(dazzle_framebuffer_t));
 
     if (ctx->renderer_data == NULL)
     {
         alloc.free(ctx);
         return NULL;
     };
 
     memcpy(ctx->renderer_data, fb, sizeof(dazzle_framebuffer_t));
 
     ctx->clear = dazzle_fb_clear;
     ctx->draw_element = dazzle_fb_draw_element;
 
     return ctx;
 }
 #endif
 
 #endif // __FRAMEBUF_INC_C__