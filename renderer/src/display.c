#include "display.h"
#include "mesh.h"
#include "triangle.h"
#include "vector.h"

App app;
uint32_t *color_buffer = NULL;
SDL_Texture *color_buffer_texture = NULL;
float fov = 640;
float delta_time = 0;
uint32_t previous_frame_time = 0;

Mesh *fea_mesh = NULL;
vec3_t camera_position = {.x = 0, .y = 0, .z = -5};
float rotation = 0.0;

vec3_t cube_rotation = {0, 0, 0};

triangle_t triangles_to_render[N_MESH_FACES];

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

void setup(void) {
    color_buffer =
        (uint32_t *)malloc(sizeof(uint32_t) * SCREEN_HEIGHT * SCREEN_WIDTH);

    color_buffer_texture = SDL_CreateTexture(
        app.renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
        SCREEN_WIDTH, SCREEN_HEIGHT

    );
}

void update(void) {
    // frame timer
    int frame_start = SDL_GetTicks();

    // frame cap
    int frame_time = SDL_GetTicks() - frame_start;
    int time_to_wait = FRAME_TARGET_TIME - frame_time;
    delta_time = (SDL_GetTicks() - previous_frame_time) / 1000.0f;

    if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME)
        SDL_Delay(time_to_wait);

    // Physics
    cube_rotation.x += 0.01;
    cube_rotation.y += 0.01;
    cube_rotation.z += 0.01;

    // Loop all triangle faces of our mesh
    for (int i = 0; i < N_MESH_FACES; i++) {
        face_t mesh_face = mesh_faces[i];

        vec3_t face_vertices[3];
        face_vertices[0] = mesh_vertices[mesh_face.a - 1];
        face_vertices[1] = mesh_vertices[mesh_face.b - 1];
        face_vertices[2] = mesh_vertices[mesh_face.c - 1];

        triangle_t projected_triangle;

        // Loop all three vertices of this current face and apply
        // transformations
        for (int j = 0; j < 3; j++) {
            vec3_t transformed_vertex = face_vertices[j];

            transformed_vertex =
                vec3_rotate_x(transformed_vertex, cube_rotation.x);
            transformed_vertex =
                vec3_rotate_y(transformed_vertex, cube_rotation.y);
            transformed_vertex =
                vec3_rotate_z(transformed_vertex, cube_rotation.z);

            // Translate the vertex away from the camera
            transformed_vertex.z -= camera_position.z;

            // Project the current vertex
            vec2_t projected_point = project(transformed_vertex);

            // Scale and translate the projected points to the middle of the
            // screen
            projected_point.x += (SCREEN_WIDTH / 2);
            projected_point.y += (SCREEN_HEIGHT / 2);

            projected_triangle.points[j] = projected_point;
        }

        // Save the projected triangle in the array of triangles to render
        triangles_to_render[i] = projected_triangle;
    }
}

void render(void) {
    SDL_SetRenderDrawColor(app.renderer, 96, 128, 255, 255);
    SDL_RenderClear(app.renderer);

    clear_color_buffer(0xFF1E1E1E);
    draw_grid();

    // Loop all projected triangles and render them
    for (int i = 0; i < N_MESH_FACES; i++) {
        triangle_t triangle = triangles_to_render[i];
        draw_rect(triangle.points[0].x, triangle.points[0].y, 3, 3, 0xFFFFFF00);
        draw_rect(triangle.points[1].x, triangle.points[1].y, 3, 3, 0xFFFFFF00);
        draw_rect(triangle.points[2].x, triangle.points[2].y, 3, 3, 0xFFFFFF00);
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
    // 1. Draw Vertical Lines
    for (int x = 0; x < SCREEN_WIDTH; x += 25) {
        for (int y = 0; y < SCREEN_HEIGHT; y++) {
            color_buffer[(SCREEN_WIDTH * y) + x] = 0xFF111111;
        }
    }

    // 2. Draw Horizontal Lines
    for (int y = 0; y < SCREEN_HEIGHT; y += 25) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            color_buffer[(SCREEN_WIDTH * y) + x] = 0xFF111111;
        }
    }
}

void draw_rect(int x, int y, int width, int height, uint32_t color) {
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            int current_x = x + i;
            int current_y = y + j;

            if (current_x >= 0 && current_x < SCREEN_WIDTH && current_y >= 0 &&
                current_y < SCREEN_HEIGHT) {
                color_buffer[(SCREEN_WIDTH * current_y) + current_x] = color;
            }
        }
    }
}

void draw_pixel(int x, int y, uint32_t color) {
    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT)
        color_buffer[(SCREEN_WIDTH * y) + x] = color;
}

Mesh *load_preset_cube_mesh() {
    Mesh *cube_mesh;
    for (int i = 0; i < N_MESH_FACES; i++) {
        cube_mesh->faces[i] = mesh_faces[i];

        face_t mesh_face = mesh_faces[i];
        vec3_t face_vertices[3];
        face_vertices[0] = mesh_vertices[mesh_face.a - 1];
        face_vertices[1] = mesh_vertices[mesh_face.b - 1];
        face_vertices[2] = mesh_vertices[mesh_face.c - 1];

        for (int j = 0; j < 3; j++) {
            cube_mesh->nodes[i].position = face_vertices[j];
        }
    }
    return cube_mesh;
}

/* -------------------------------------------------------------------------- */
/*                                 PROJECTION                                 */
/* -------------------------------------------------------------------------- */
vec2_t project(vec3_t point) {
    vec2_t projected_point = {.x = (fov * point.x) / point.z,
                              .y = (fov * point.y) / point.z};
    return projected_point;
}

void cleanup(void) {
    free(color_buffer);
    SDL_DestroyRenderer(app.renderer);
    SDL_DestroyWindow(app.window);
    SDL_Quit();
}