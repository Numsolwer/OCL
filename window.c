#include <SDL3/SDL.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
    int width;
    int height;
    bool running;
    Uint8 bg_r, bg_g, bg_b;
    Uint32 frame_delay;
    Uint32 last_frame_time;
} OCL2DRI_Context;

EXPORT OCL2DRI_Context* ocl2dri_init(int width, int height, const char* title) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
        return NULL;
    }

    OCL2DRI_Context* ctx = (OCL2DRI_Context*)malloc(sizeof(OCL2DRI_Context));
    if (!ctx) {
        SDL_Quit();
        return NULL;
    }

    ctx->window = SDL_CreateWindow(title, width, height, SDL_WINDOW_RESIZABLE);
    if (!ctx->window) {
        free(ctx);
        SDL_Quit();
        return NULL;
    }

    ctx->renderer = SDL_CreateRenderer(ctx->window, NULL);
    if (!ctx->renderer) {
        SDL_DestroyWindow(ctx->window);
        free(ctx);
        SDL_Quit();
        return NULL;
    }

    SDL_SetRenderVSync(ctx->renderer, 1);

    ctx->width = width;
    ctx->height = height;
    ctx->running = true;
    ctx->bg_r = 0;
    ctx->bg_g = 0;
    ctx->bg_b = 0;
    ctx->frame_delay = 16;
    ctx->last_frame_time = SDL_GetTicks();

    return ctx;
}

EXPORT void ocl2dri_set_background(OCL2DRI_Context* ctx, Uint8 r, Uint8 g, Uint8 b) {
    if (!ctx || !ctx->renderer) return;
    ctx->bg_r = r;
    ctx->bg_g = g;
    ctx->bg_b = b;
}

EXPORT void ocl2dri_set_title(OCL2DRI_Context* ctx, const char* title) {
    if (!ctx || !ctx->window) return;
    SDL_SetWindowTitle(ctx->window, title);
}

EXPORT void ocl2dri_set_size(OCL2DRI_Context* ctx, int width, int height) {
    if (!ctx || !ctx->window) return;
    SDL_SetWindowSize(ctx->window, width, height);
    ctx->width = width;
    ctx->height = height;
}

EXPORT void ocl2dri_set_position(OCL2DRI_Context* ctx, int x, int y) {
    if (!ctx || !ctx->window) return;
    SDL_SetWindowPosition(ctx->window, x, y);
}

EXPORT void ocl2dri_set_fullscreen(OCL2DRI_Context* ctx, bool fullscreen) {
    if (!ctx || !ctx->window) return;
    SDL_SetWindowFullscreen(ctx->window, fullscreen);
}

EXPORT void ocl2dri_set_opacity(OCL2DRI_Context* ctx, float opacity) {
    if (!ctx || !ctx->window) return;
    if (opacity < 0.0f) opacity = 0.0f;
    if (opacity > 1.0f) opacity = 1.0f;
    SDL_SetWindowOpacity(ctx->window, opacity);
}

EXPORT void ocl2dri_set_border(OCL2DRI_Context* ctx, bool bordered) {
    if (!ctx || !ctx->window) return;
    SDL_SetWindowBordered(ctx->window, bordered);
}

EXPORT void ocl2dri_set_min_size(OCL2DRI_Context* ctx, int min_width, int min_height) {
    if (!ctx || !ctx->window) return;
    SDL_SetWindowMinimumSize(ctx->window, min_width, min_height);
}

EXPORT void ocl2dri_set_max_size(OCL2DRI_Context* ctx, int max_width, int max_height) {
    if (!ctx || !ctx->window) return;
    SDL_SetWindowMaximumSize(ctx->window, max_width, max_height);
}

EXPORT void ocl2dri_set_always_on_top(OCL2DRI_Context* ctx, bool on_top) {
    if (!ctx || !ctx->window) return;
    SDL_SetWindowAlwaysOnTop(ctx->window, on_top);
}

EXPORT void ocl2dri_set_resizable(OCL2DRI_Context* ctx, bool resizable) {
    if (!ctx || !ctx->window) return;
    SDL_SetWindowResizable(ctx->window, resizable);
}

EXPORT void ocl2dri_set_frame_rate(OCL2DRI_Context* ctx, int fps) {
    if (!ctx || fps <= 0) return;
    ctx->frame_delay = 1000 / fps;
}

EXPORT void ocl2dri_hide(OCL2DRI_Context* ctx) {
    if (!ctx || !ctx->window) return;
    SDL_HideWindow(ctx->window);
}

EXPORT void ocl2dri_show(OCL2DRI_Context* ctx) {
    if (!ctx || !ctx->window) return;
    SDL_ShowWindow(ctx->window);
}

EXPORT void ocl2dri_set_icon(OCL2DRI_Context* ctx, const char* icon_path) {
    if (!ctx || !ctx->window || !icon_path) return;
    SDL_Surface* icon = SDL_LoadBMP(icon_path);
    if (!icon) {
        // Error logged in interpreter.py, no printf here
        return;
    }
    SDL_SetWindowIcon(ctx->window, icon);
    SDL_DestroySurface(icon);
}

EXPORT void ocl2dri_get_mouse_position(OCL2DRI_Context* ctx, float* x, float* y) {
    if (!ctx || !x || !y) return;
    SDL_GetMouseState(x, y);
}

EXPORT int ocl2dri_get_mouse_button_state(OCL2DRI_Context* ctx, int button) {
    if (!ctx) return 0;
    float x, y;
    Uint32 state = SDL_GetMouseState(&x, &y);
    switch (button) {
        case 1: return (state & SDL_BUTTON_LMASK) != 0;
        case 2: return (state & SDL_BUTTON_MMASK) != 0;
        case 3: return (state & SDL_BUTTON_RMASK) != 0;
        default: return 0;
    }
}

EXPORT float ocl2dri_get_delta_time(OCL2DRI_Context* ctx) {
    if (!ctx) return 0.0f;
    Uint32 current_time = SDL_GetTicks();
    float delta_time = (current_time - ctx->last_frame_time) / 1000.0f;
    ctx->last_frame_time = current_time;
    return delta_time;
}

EXPORT void ocl2dri_update(OCL2DRI_Context* ctx) {
    if (!ctx || !ctx->renderer) return;

    Uint32 frame_start = SDL_GetTicks();

    SDL_SetRenderDrawColor(ctx->renderer, ctx->bg_r, ctx->bg_g, ctx->bg_b, 255);
    SDL_RenderClear(ctx->renderer);
    SDL_RenderPresent(ctx->renderer);

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            ctx->running = false;
        }
    }

    Uint32 frame_time = SDL_GetTicks() - frame_start;
    if (frame_time < ctx->frame_delay) {
        SDL_Delay(ctx->frame_delay - frame_time);
    }
}

EXPORT bool ocl2dri_is_running(OCL2DRI_Context* ctx) {
    if (!ctx) return false;
    return ctx->running;
}

EXPORT void ocl2dri_destroy(OCL2DRI_Context* ctx) {
    if (!ctx) return;
    if (ctx->renderer) SDL_DestroyRenderer(ctx->renderer);
    if (ctx->window) SDL_DestroyWindow(ctx->window);
    free(ctx);
    SDL_Quit();
}

EXPORT int ocl2dri_get_key_state(OCL2DRI_Context* ctx, const char* key) {
    if (!ctx) return 0;
    const Uint8* state = (const Uint8*)SDL_GetKeyboardState(NULL);
    SDL_Scancode scancode = SDL_GetScancodeFromName(key);
    if (scancode == SDL_SCANCODE_UNKNOWN) {
        return 0;  // Error logged in interpreter.py if needed
    }
    return state[scancode];
}