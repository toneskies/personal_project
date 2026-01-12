#include "vector.h"
#include <math.h>

vec3_t vec3_rotate_z(vec3_t v, float angle) {
    vec3_t rotate_z = {.x = v.x * cos(angle) - v.y * sin(angle),
                       .y = v.x * sin(angle) + v.y * cos(angle),
                       .z = v.z};
    return rotate_z;
}

vec3_t vec3_rotate_x(vec3_t v, float angle) {
    vec3_t rotate_x = {.x = v.x,
                       .y = v.y * cos(angle) - v.z * sin(angle),
                       .z = v.y * sin(angle) + v.z * cos(angle)};
    return rotate_x;
}

vec3_t vec3_rotate_y(vec3_t v, float angle) {
    vec3_t rotate_y = {.x = v.x * cos(angle) - v.z * sin(angle),
                       .y = v.y,
                       .z = v.x * sin(angle) + v.z * cos(angle)};
    return rotate_y;
}

vec3_t vec3_sub(vec3_t a, vec3_t b) {
    vec3_t result = {.x = a.x - b.x, .y = a.y - b.y, .z = a.z - b.z};
    return result;
}
vec3_t vec3_cross(vec3_t a, vec3_t b) {
    vec3_t result = {.x = a.y * b.z - a.z * b.y,
                     .y = a.z * b.x - a.x * b.z,
                     .z = a.x * b.y - a.y * b.x};
    return result;
}

float vec3_dot(vec3_t a, vec3_t b) {
    return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}
void vec3_normalize(vec3_t *v) {
    float length = sqrt(v->x * v->x + v->y * v->y + v->z * v->z);
    v->x /= length;
    v->y /= length;
    v->z /= length;
}

vec4_t vec4_from_vec3(vec3_t v) { return (vec4_t){v.x, v.y, v.z, 1.0}; }

vec3_t vec3_from_vec4(vec4_t v) { return (vec3_t){v.x, v.y, v.z}; }