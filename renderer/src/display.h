#ifndef DISPLAY_H
#define DISPLAY_H

#include "SDL2/SDL.h"
#include "mesh.h"
#include "vector.h"

/* -------------------------------------------------------------------------- */
/*                             STRUCT DEFINITIONS                             */
/* -------------------------------------------------------------------------- */
typedef struct {
    SDL_Renderer *renderer;
    SDL_Window *window;
} App;

/* -------------------------------------------------------------------------- */
/*                                   GLOBALS                                  */
/* -------------------------------------------------------------------------- */
extern App app;
extern uint32_t *color_buffer;
extern SDL_Texture *color_buffer_texture;

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

#define FPS 60
#define FRAME_TARGET_TIME (1000 / FPS)

/* -------------------------------------------------------------------------- */
/*                                SDL FUNCTIONS                               */
/* -------------------------------------------------------------------------- */
void initSDL(void);
void doInput(void);
void setup(void);
void update(void);
void render(void);
void clear_color_buffer(uint32_t color);
void render_color_buffer(void);

/* -------------------------------------------------------------------------- */
/*                               DRAW FUNCTIONS                               */
/* -------------------------------------------------------------------------- */
void draw_grid(void);
void draw_rect(int x, int y, int width, int height, uint32_t color);
void draw_pixel(int x, int y, uint32_t color);
Mesh *load_preset_cube_mesh();

/* --------------------------------------------------------------------------*/
/*                                                                           */
/*                                 PROJECTION                                */
/* --------------------------------------------------------------------------*/
vec2_t project(vec3_t point);

// CLEAN UP for the renderer on exit
void cleanup(void);

#endif