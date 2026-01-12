#include "display.h"
#include "mesh.h"
#include "vector.h"

App app;
uint32_t *color_buffer = NULL;
SDL_Texture *color_buffer_texture = NULL;
float fov = 640;

Mesh *fea_mesh = NULL;
vec3_t camera_position = {.x = 0, .y = 0, .z = -5};
float rotation = 0.0;

float previous_frame_time;

/* -------------------------------------------------------------------------- */
/*                                SDL FUNCTIONS                               */
/* -------------------------------------------------------------------------- */
void initSDL(void) {

    int rendererFlags, windowFlags;

    rendererFlags = SDL_RENDERER_ACCELERATED;

    windowFlags = 0;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Couldn't initialize SDL: %s\n", SDL_GetError());
        exit(1);
    }

    app.window = SDL_CreateWindow("Eno", SDL_WINDOWPOS_UNDEFINED,
                                  SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
                                  SCREEN_HEIGHT, windowFlags);

    if (!app.window) {
        printf("Failed to open %d x %d window: %s\n", SCREEN_WIDTH,
               SCREEN_HEIGHT, SDL_GetError());
        exit(1);
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

    app.renderer = SDL_CreateRenderer(app.window, -1, rendererFlags);

    if (!app.renderer) {
        printf("Failed to create renderer: %s\n", SDL_GetError());
        exit(1);
    }

    color_buffer =
        (uint32_t *)malloc(sizeof(uint32_t) * SCREEN_HEIGHT * SCREEN_WIDTH);

    color_buffer_texture = SDL_CreateTexture(
        app.renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
        SCREEN_WIDTH, SCREEN_HEIGHT

    );
}

void doInput(void) {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            exit(0);
            break;
        case SDL_KEYDOWN:
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                exit(0);
                break;
            }

        default:
            break;
        }
    }
}

void setup(void) { fea_mesh = load_generated_mesh(); }

void update(void) {
    int time_to_wait =
        FRAME_TARGET_TIME - (SDL_GetTicks() - previous_frame_time);

    if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME)
        SDL_Delay(time_to_wait);

    previous_frame_time = SDL_GetTicks();
}

void render(void) {
    SDL_SetRenderDrawColor(app.renderer, 96, 128, 255, 255);
    SDL_RenderClear(app.renderer);

    clear_color_buffer(0xFF1E1E1E);
    draw_grid();

    if (fea_mesh) {
        draw_mesh_nodes(fea_mesh);
    }

    render_color_buffer();

    SDL_RenderPresent(app.renderer);
}

void clear_color_buffer(uint32_t color) {
    for (int x = 0; x < SCREEN_WIDTH * SCREEN_HEIGHT; x++)
        color_buffer[x] = color;
}

void render_color_buffer(void) {
    SDL_UpdateTexture(color_buffer_texture, NULL, color_buffer,
                      (int)(SCREEN_WIDTH * sizeof(uint32_t)));
    SDL_RenderCopy(app.renderer, color_buffer_texture, NULL, NULL);
}

/* -------------------------------------------------------------------------- */
/*                               DRAW FUNCTIONS                               */
/* -------------------------------------------------------------------------- */
void draw_grid(void) {
    for (int y = 0; y < SCREEN_HEIGHT; y++)
        for (int x = 0; x < SCREEN_WIDTH; x++)
            if (x % 25 == 0 || y % 25 == 0)
                color_buffer[(SCREEN_WIDTH * y) + x] = 0xFF111111;
}

void draw_rect(int x, int y, int width, int height, uint32_t color) {
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            int current_x = x + i;
            int current_y = y + j;
            draw_pixel(current_x, current_y, color);
        }
    }
}

void draw_pixel(int x, int y, uint32_t color) {
    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT)
        color_buffer[(SCREEN_WIDTH * y) + x] = color;
}

void draw_mesh_nodes(Mesh *mesh) {
    rotation += 0.001;
    for (int i = 0; i < mesh->num_nodes; i++) {
        vec3_t point = mesh->nodes[i].position;

        vec3_t transformed_point = vec3_rotate_x(point, rotation);
        transformed_point = vec3_rotate_y(transformed_point, rotation);
        transformed_point = vec3_rotate_z(transformed_point, rotation);

        vec2_t projected_point = project(transformed_point);

        draw_rect(projected_point.x, projected_point.y, 4, 4, 0xFFFFFF00);
    }
}

/* -------------------------------------------------------------------------- */
/*                                 PROJECTION                                 */
/* -------------------------------------------------------------------------- */
vec2_t project(vec3_t point) {
    float perspective_factor = fov / (point.z - camera_position.z);
    vec2_t projected_point = {
        .x = (point.x * perspective_factor) + (SCREEN_WIDTH / 2),
        .y = (point.y * perspective_factor) + (SCREEN_HEIGHT / 2),
    };
    return projected_point;
}

void cleanup(void) {
    free(color_buffer);
    SDL_DestroyRenderer(app.renderer);
    SDL_DestroyWindow(app.window);
    SDL_Quit();
}