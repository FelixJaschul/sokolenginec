#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"

#include <SDL3/SDL.h>

#define SOKOL_IMPL
#include "sokol_gfx.h"
#include "sokol_log.h"

#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>

#define ASSERT(cond) do { \
    if (!(cond)) { \
        fprintf(stderr, "ASSERTION FAILED: %s at %s:%d\n", #cond, __FILE__, __LINE__); \
        abort(); \
    } \
} while(0)

typedef struct {
    SDL_Window* window;
    SDL_GLContext gl_context;
    int width;
    int height;
    float clear_color[3];
    int frame_count;
    bool running;
} state;

static void app_init(state* s) {
    ASSERT(s);
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        exit(1);
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    s->width = 1280;
    s->height = 720;
    s->window = SDL_CreateWindow(
        "SDL3 + Sokol + CImGui",
        s->width, s->height,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY
    );
    ASSERT(s->window);

    s->gl_context = SDL_GL_CreateContext(s->window);
    ASSERT(s->gl_context);

    SDL_GL_SetSwapInterval(1);

    sg_setup(&(sg_desc){
        .environment = {
            .defaults = {
                .color_format = SG_PIXELFORMAT_RGBA8,
                .depth_format = SG_PIXELFORMAT_DEPTH_STENCIL,
                .sample_count = 1,
            }
        },
        .logger.func = slog_func,
    });
    ASSERT(sg_isvalid());

    s->clear_color[0] = 0.45f;
    s->clear_color[1] = 0.55f;
    s->clear_color[2] = 0.60f;
    s->frame_count = 0;
    s->running = true;

    printf("Application initialized\n");
    printf("Backend: %s\n", sg_query_backend() == SG_BACKEND_GLCORE ? "OpenGL Core" : "Unknown");
}

static void app_events(state* s) {
    ASSERT(s);
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            s->running = false;
        }
        if (event.type == SDL_EVENT_KEY_DOWN) {
            if (event.key.key == SDLK_ESCAPE) {
                s->running = false;
            }
            if (event.key.key == SDLK_SPACE) {
                printf("Frame %d, Size: %dx%d, Color: (%.2f, %.2f, %.2f)\n", 
                       s->frame_count, s->width, s->height, 
                       s->clear_color[0], s->clear_color[1], s->clear_color[2]);
            }
        }
    }
}

static void app_update(state* s) {
    ASSERT(s);
    SDL_GetWindowSizeInPixels(s->window, &s->width, &s->height);

    float t = (float)s->frame_count * 0.01f;
    s->clear_color[0] = 0.45f + 0.2f * sinf(t);
    s->clear_color[1] = 0.55f + 0.2f * sinf(t * 1.3f);
    s->clear_color[2] = 0.60f + 0.2f * sinf(t * 1.7f);
}

static void app_render(state* s) {
    ASSERT(s);
    sg_pass pass = {
        .action = {
            .colors[0] = {
                .load_action = SG_LOADACTION_CLEAR,
                .clear_value = { s->clear_color[0], s->clear_color[1], s->clear_color[2], 1.0f }
            }
        },
        .swapchain = {
            .width = s->width,
            .height = s->height,
            .sample_count = 1,
            .color_format = SG_PIXELFORMAT_RGBA8,
            .depth_format = SG_PIXELFORMAT_DEPTH_STENCIL,
            .gl = { .framebuffer = 0 }
        }
    };
    
    sg_begin_pass(&pass);
    sg_end_pass();
    sg_commit();
    
    SDL_GL_SwapWindow(s->window);
    s->frame_count++;
}

static void app_shutdown(state* s) {
    ASSERT(s);
    sg_shutdown();
    SDL_GL_DestroyContext(s->gl_context);
    SDL_DestroyWindow(s->window);
    SDL_Quit();
    printf("Cleanup complete\n");
}

int main(int argc, char* argv[]) {
    (void)argc; (void)argv;
    state s = {0};
    app_init(&s);
    
    printf("Application running. Press ESC to exit.\n");
    printf("You should see an animated colored background.\n");
    
    while (s.running) {
        app_events(&s);
        app_update(&s);
        app_render(&s);
    }
    app_shutdown(&s);
    return 0;
}

