#define __DAZZLE_IMPL__

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include <betterm/bt.h>
#include <SDL2/SDL.h>

int main(int argc, char **argv) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Dazzle Test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 800, 600);

    dazzle_allocator_t alloc;
    alloc.malloc = malloc;
    alloc.free = free;

    dazzle_framebuffer_t fb;
    fb.address         = (uintptr_t)malloc(800 * 600 * 4);
    fb.width           = 800;
    fb.height          = 600;
    fb.pitch           = 4 * 800;
    fb.bpp             = 32;
    fb.red_mask        = 0xFF; 
    fb.green_mask      = 0xFF;
    fb.blue_mask       = 0xFF;
    fb.alpha_mask      = 0xFF;
    fb.red_shift       = 24;
    fb.green_shift     = 16;
    fb.blue_shift      = 8;
    fb.alpha_shift     = 0;

    dazzle_context_t* ctx = dazzle_init_fb(alloc, &fb);

    void* blitablebuf = malloc(100 * 100 * 4);

    for(int i = 0; i < 100; i++){
        for(int j = 0; j < 100; j++){
            ((uint32_t*)blitablebuf)[i * 100 + j] = i*j*0xff;
        }
    }

    dazzle_clear(ctx, 0x00000000);

    uint64_t color = 0x00000000;
    bool done = false;
    float hue = 0.0f;

    // Used fpr performance calculations
    uint64_t last_time = SDL_GetPerformanceCounter();
    double accum_time = 0;
    double delta_time = 0;
    double fps = 0;
    
    // Statistics
    double accum_fps = 0;
    double min_fps = 10000000, max_fps = 0;
    double accum_frametime = 0;
    double min_frametime = 10000000, max_frametime = 0;

    uint64_t num_secs = 0;


    while (!done) {
        uint64_t currentTime = SDL_GetPerformanceCounter();
        delta_time = (double)(currentTime - last_time) / SDL_GetPerformanceFrequency();
        last_time = currentTime;

        // Calculate FPS
        fps = 1.0 / delta_time;

        accum_time += delta_time;
        if (accum_time >= 1.0) {
            printf("==== Sample #%3d ====\n", num_secs);
            printf("FPS: %.2f\n", fps);
            printf("Frametime: %.6f\n", delta_time);
            accum_fps += fps;
            num_secs++;
            fflush(stdout);
            accum_time = 0;
            min_fps = fmin(min_fps, fps);
            max_fps = fmax(max_fps, fps);

            accum_frametime += delta_time;
            min_frametime = fmin(min_frametime, delta_time);
            max_frametime = fmax(max_frametime, delta_time);
        }

        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                done = true;
                break;
            }
        }

        SDL_UpdateTexture(texture, NULL, (void*)fb.address, fb.pitch);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

    printf("==== Final Results ====\n");
    printf("Final Average FPS: %.2f\n", accum_fps / num_secs);
    printf("Minimum FPS: %.2f\n", min_fps);
    printf("Maximum FPS: %.2f\n", max_fps);
    printf("Final Average Frame Time: %.6f\n", accum_frametime / num_secs);
    printf("Minimum Frame Time: %.6f\n", min_frametime);
    printf("Maximum Frame Time: %.6f\n", max_frametime);
    printf("Score(total frames pushed): %.2f\n", accum_fps);
    printf("Measurement duration: %d seconds\n", num_secs);

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}