#include <SDL3/SDL.h>

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"

#define SOKOL_IMPL
#include "sokol_gfx.h"
#include "sokol_log.h"

#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>

#define ASSERT(cond) do { if (!(cond)) { fprintf(stderr, "ASSERTION FAILED: %s at %s:%d\n", #cond, __FILE__, __LINE__); abort(); } } while(0)

typedef struct {
    SDL_Window* window;
    SDL_GLContext gl_context;
    int width;
    int height;
    float clear_color[3];
    int frame_count;
    bool running;
} state_t;

state_t state;

#define WIDTH  800
#define HEIGHT 600
#define TITLE  "TEST"

#ifndef M_PI
#define M_PI 3.14159265359 
#endif // M_PI

// Easy call for func
#define Run()       app_run()
#define Start()     app_run()
#define Init()      app_init()
#define Events()    app_events()
#define Frame()     app_update()
#define Update()    app_update()
#define Render()    app_render()
#define Deinit()    app_deinit()
#define Shutdown()  app_deinit()
#define Cleanup()   app_deinit()

void app_init() 
{
    ASSERT(SDL_Init(SDL_INIT_VIDEO));

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    state.width  = WIDTH;
    state.height = HEIGHT;
    state.window = 
        SDL_CreateWindow(
            TITLE, 
            state.width, 
            state.height, 
            SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY
        );
    ASSERT(state.window);

    state.gl_context = SDL_GL_CreateContext(state.window);
    ASSERT(state.gl_context);

    SDL_GL_SetSwapInterval(1);

    sg_setup(&(sg_desc) {
        .environment  = {
            .defaults = {
                .color_format = SG_PIXELFORMAT_RGBA8,
                .depth_format = SG_PIXELFORMAT_DEPTH_STENCIL,
                .sample_count = 1,
            }
        },
        .logger.func = slog_func,
    });
    ASSERT(sg_isvalid());

    state.clear_color[0] = 0.45f;
    state.clear_color[1] = 0.55f;
    state.clear_color[2] = 0.60f;
    state.frame_count    = 0;
    state.running        = true;
}

void app_events() 
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) 
    {
        if (event.type == SDL_EVENT_QUIT) state.running = false;
        if (event.type == SDL_EVENT_KEY_DOWN) 
        {
            if (event.key.key == SDLK_ESCAPE) state.running = false;
        }
    }
}

void app_update() 
{
    SDL_GetWindowSizeInPixels(state.window, &state.width, &state.height);

    float t = (float)state.frame_count * 0.01f;
    state.clear_color[0] = 0.45f + 0.2f * sinf(t);
    state.clear_color[1] = 0.55f + 0.2f * sinf(t * 1.3f);
    state.clear_color[2] = 0.60f + 0.2f * sinf(t * 1.7f);
}

void app_render() 
{
    sg_pass pass = {
        .action  = {
            .colors[0] = {
                .load_action = SG_LOADACTION_CLEAR,
                .clear_value = { 
                    state.clear_color[0], 
                    state.clear_color[1], 
                    state.clear_color[2], 1.0f 
                }
            }
        },
        .swapchain = {
            .width  = state.width,
            .height = state.height,
            .sample_count = 1,
            .color_format = SG_PIXELFORMAT_RGBA8,
            .depth_format = SG_PIXELFORMAT_DEPTH_STENCIL,
            .gl = { 
                .framebuffer = 0 
            }
        }
    };
    
    sg_begin_pass(&pass);
    sg_end_pass();
    sg_commit();
    
    SDL_GL_SwapWindow(state.window);
    state.frame_count++;
}

void app_deinit() 
{
    sg_shutdown();
    SDL_GL_DestroyContext(state.gl_context);
    SDL_DestroyWindow(state.window);
    SDL_Quit();
}

void app_run() 
{
    Init();
    while (state.running)
    {
        Events();
        Update();
        Render();
    }
    Deinit();
}

