#include "display.h"
#include "light.h"
#include "matrix.h"
#include "mesh.h"
#include "triangle.h"
#include "vector.h"
#include <stdlib.h>

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

App app;
uint32_t *color_buffer = NULL;
SDL_Texture *color_buffer_texture = NULL;
float fov = 640;
float delta_time = 0;
uint32_t previous_frame_time = 0;

vec3_t camera_position = {.x = 0, .y = 0, .z = -5};
vec3_t cube_rotation = {0, 0, 0};

triangle_t *triangles_to_render = NULL;

mat4_t projection_matrix;

/* -------------------------------------------------------------------------- */
/*                                   HELPERS                                  */
/* -------------------------------------------------------------------------- */
void int_swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

int compare_triangle_depth(const void *a, const void *b) {
    triangle_t *t1 = (triangle_t *)a;
    triangle_t *t2 = (triangle_t *)b;

    if (t1->avg_depth < t2->avg_depth)
        return 1; // t1 is closer, so it goes later (painted last)
    else if (t1->avg_depth > t2->avg_depth)
        return -1; // t1 is farther, so it goes first (painted first)

    return 0;
}

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

void setup(Mesh *mesh) {
    color_buffer =
        (uint32_t *)malloc(sizeof(uint32_t) * SCREEN_HEIGHT * SCREEN_WIDTH);

    color_buffer_texture = SDL_CreateTexture(
        app.renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
        SCREEN_WIDTH, SCREEN_HEIGHT

    );

    float fov = 3.141592 / 3.0; // 60 degrees in radians
    float aspect = (float)SCREEN_HEIGHT / (float)SCREEN_WIDTH;
    float znear = 0.1;
    float zfar = 100.0;

    projection_matrix = mat4_make_perspective(fov, aspect, znear, zfar);

    *mesh = load_mesh("./assets/f22.obj");
}

void update(Mesh *mesh) {
    // frame timer
    int frame_start = SDL_GetTicks();

    // frame cap
    int frame_time = SDL_GetTicks() - frame_start;
    int time_to_wait = FRAME_TARGET_TIME - frame_time;
    delta_time = (SDL_GetTicks() - previous_frame_time) / 1000.0f;

    if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME)
        SDL_Delay(time_to_wait);

    previous_frame_time = SDL_GetTicks();

    // Physics
    mesh->rotation.x += 0.01;
    mesh->rotation.y += 0.01;
    mesh->rotation.z += 0.01;

    arrfree(triangles_to_render);
    triangles_to_render = NULL;

    int num_faces = arrlen(mesh->faces);

    // Loop all triangle faces of our mesh
    for (int i = 0; i < num_faces; i++) {
        face_t mesh_face = mesh->faces[i];

        vec3_t face_vertices[3];
        face_vertices[0] = mesh->vertices[mesh_face.a];
        face_vertices[1] = mesh->vertices[mesh_face.b];
        face_vertices[2] = mesh->vertices[mesh_face.c];

        vec3_t transformed_vertices[3];
        triangle_t projected_triangle;

        for (int j = 0; j < 3; j++) {
            vec3_t transformed_vertex = face_vertices[j];

            // 1. Model Transform (Rotate/Translate)
            transformed_vertex =
                vec3_rotate_x(transformed_vertex, mesh->rotation.x);
            transformed_vertex =
                vec3_rotate_y(transformed_vertex, mesh->rotation.y);
            transformed_vertex =
                vec3_rotate_z(transformed_vertex, mesh->rotation.z);

            transformed_vertex.z -= 5.0; // Move away from camera

            // 2. PROJECTION MATRIX (View Space -> Clip Space)
            vec4_t projected_point = mat4_mul_vec4(
                projection_matrix, vec4_from_vec3(transformed_vertex));

            // 3. PERSPECTIVE DIVIDE (Clip Space -> NDC)
            if (projected_point.w != 0) {
                projected_point.x /= projected_point.w;
                projected_point.y /= projected_point.w;
                projected_point.z /= projected_point.w;
            }

            // 4. VIEWPORT TRANSFORM (NDC -> Screen Space)
            // NDC is [-1, 1], we map to [0, Width]
            projected_point.x = (projected_point.x + 1.0) * 0.5 * SCREEN_WIDTH;
            projected_point.y = (projected_point.y + 1.0) * 0.5 * SCREEN_HEIGHT;

            projected_triangle.points[j].x = projected_point.x;
            projected_triangle.points[j].y = projected_point.y;
        }
        // Backface Culling
        vec3_t vector_ab =
            vec3_sub(transformed_vertices[1], transformed_vertices[0]);
        vec3_t vector_ac =
            vec3_sub(transformed_vertices[2], transformed_vertices[0]);

        // Compute Normal (perpendicular to the face)
        vec3_t normal = vec3_cross(vector_ab, vector_ac);
        vec3_normalize(&normal);

        vec3_t camera_ray = vec3_sub(camera_position, transformed_vertices[0]);

        // Calculate alignment
        float dot_normal_camera = vec3_dot(normal, camera_ray);

        if (dot_normal_camera < 0) {
            continue;
        }

        // LIGHTING
        vec3_t light_direction = get_light_direction();
        float alignment = -vec3_dot(normal, light_direction);
        uint32_t base_color = mesh_face.color;

        float avg_depth =
            (transformed_vertices[0].z + transformed_vertices[1].z +
             transformed_vertices[2].z) /
            3.0;
        for (int j = 0; j < 3; j++) {

            vec2_t projected_point = project(transformed_vertices[j]);

            projected_point.x += (SCREEN_WIDTH / 2);
            projected_point.y += (SCREEN_HEIGHT / 2);

            projected_triangle.points[j] = projected_point;
            projected_triangle.color =
                light_apply_intensity(base_color, alignment);
            projected_triangle.avg_depth = avg_depth;
        }

        int num_triangles = arrlen(triangles_to_render);
        qsort(triangles_to_render, num_triangles, sizeof(triangle_t),
              compare_triangle_depth);
        arrput(triangles_to_render, projected_triangle);
    }
}

void render(Mesh *mesh) {
    SDL_SetRenderDrawColor(app.renderer, 96, 128, 255, 255);
    SDL_RenderClear(app.renderer);

    clear_color_buffer(0xFF1E1E1E);
    draw_grid();

    int num_triangles = arrlen(triangles_to_render);

    // Loop all projected triangles and render them
    for (int i = 0; i < num_triangles; i++) {
        triangle_t triangle = triangles_to_render[i];

        // Use your new draw_triangle function
        draw_filled_triangle(triangle.points[0].x, triangle.points[0].y,
                             triangle.points[1].x, triangle.points[1].y,
                             triangle.points[2].x, triangle.points[2].y,
                             triangle.color);
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

void draw_line(int x0, int y0, int x1, int y1, uint32_t color) {
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);

    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;

    int err = dx - dy;

    while (1) {
        draw_pixel(x0, y0, color);

        if (x0 == x1 && y0 == y1)
            break;

        int e2 = 2 * err;

        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }

        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}
void draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2,
                   uint32_t color) {
    draw_line(x0, y0, x1, y1, color);
    draw_line(x1, y1, x2, y2, color);
    draw_line(x2, y2, x0, y0, color);
}

// Draw a filled triangle with a flat bottom
// (Top vertex is (x0,y0), bottom base is (x1,y1)-(x2,y2))
void fill_flat_bottom_triangle(int x0, int y0, int x1, int y1, int x2, int y2,
                               uint32_t color) {
    // Find the two slopes (inverse slope: run/rise)
    float inv_slope1 = (float)(x1 - x0) / (y1 - y0);
    float inv_slope2 = (float)(x2 - x0) / (y2 - y0);

    // Start x_start and x_end from the top vertex
    float x_start = x0;
    float x_end = x0;

    // Loop all the scanlines from top to bottom
    for (int y = y0; y <= y1; y++) {
        draw_line((int)x_start, y, (int)x_end, y, color);
        x_start += inv_slope1;
        x_end += inv_slope2;
    }
}

// Draw a filled triangle with a flat top
// (Top base is (x0,y0)-(x1,y1), bottom vertex is (x2,y2))
void fill_flat_top_triangle(int x0, int y0, int x1, int y1, int x2, int y2,
                            uint32_t color) {
    // Find the two slopes (inverse slope: run/rise)
    float inv_slope1 = (float)(x2 - x0) / (y2 - y0);
    float inv_slope2 = (float)(x2 - x1) / (y2 - y1);

    // Start x_start and x_end from the bottom vertex
    float x_start = x2;
    float x_end = x2;

    // Loop all the scanlines from bottom to top
    for (int y = y2; y >= y0; y--) {
        draw_line((int)x_start, y, (int)x_end, y, color);
        x_start -= inv_slope1;
        x_end -= inv_slope2;
    }
}

void draw_filled_triangle(int x0, int y0, int x1, int y1, int x2, int y2,
                          uint32_t color) {
    // We need to sort the vertices by y-coordinate ascending (y0 < y1 < y2)
    if (y0 > y1) {
        int_swap(&y0, &y1);
        int_swap(&x0, &x1);
    }
    if (y1 > y2) {
        int_swap(&y1, &y2);
        int_swap(&x1, &x2);
    }
    if (y0 > y1) {
        int_swap(&y0, &y1);
        int_swap(&x0, &x1);
    }

    if (y1 == y2) {
        // Triangle is already a flat-bottom triangle
        fill_flat_bottom_triangle(x0, y0, x1, y1, x2, y2, color);
    } else if (y0 == y1) {
        // Triangle is already a flat-top triangle
        fill_flat_top_triangle(x0, y0, x1, y1, x2, y2, color);
    } else {
        // Calculate the midpoint (Mx, My) to split the triangle into two parts
        int My = y1;
        int Mx = ((float)((x2 - x0) * (y1 - y0)) / (float)(y2 - y0)) + x0;

        // Draw the flat-bottom part
        fill_flat_bottom_triangle(x0, y0, x1, y1, Mx, My, color);

        // Draw the flat-top part
        fill_flat_top_triangle(x1, y1, Mx, My, x2, y2, color);
    }
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