#include "mesh.h"
#include "display.h"

#define N_POINTS 9 * 9 * 9
vec3_t cube_points[N_POINTS];

vec3_t *load_generated_cube(int length, int width, int height) {

    int point_count = 0;

    for (float x = -1; x <= 1; x += 0.25) {
        for (float y = -1; y <= 1; y += 0.25) {
            for (float z = -1; z <= 1; z += 0.25) {
                vec3_t new_point = {.x = x, .y = y, .z = z};
                cube_points[point_count++] = new_point;
            }
        }
    }

    return cube_points;
}