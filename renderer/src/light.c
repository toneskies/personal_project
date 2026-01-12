#include "light.h"

static light_t global_light;

void init_light(vec3_t direction) { global_light.direction = direction; }

vec3_t get_light_direction(void) { return global_light.direction; }

uint32_t light_apply_intensity(uint32_t original_color,
                               float percentage_factor) {
    if (percentage_factor < 0)
        percentage_factor = 0;
    if (percentage_factor > 1)
        percentage_factor = 1;

    uint32_t a = (original_color & 0xFF000000);
    uint32_t r = (original_color & 0x00FF0000) >> 16;
    uint32_t g = (original_color & 0x0000FF00) >> 8;
    uint32_t b = (original_color & 0x000000FF);

    r = (uint32_t)(r * percentage_factor);
    g = (uint32_t)(g * percentage_factor);
    b = (uint32_t)(b * percentage_factor);

    return a | (r << 16) | (g << 8) | b;
}