#ifndef LIGHT_H
#define LIGHT_H

#include "vector.h"
#include <stdint.h>

typedef struct {
    vec3_t direction;
} light_t;

// Initialize the global light
void init_light(vec3_t direction);

// Get the current light direction
vec3_t get_light_direction(void);

// Change a color's brightness based on a percentage factor (0.0 to 1.0)
uint32_t light_apply_intensity(uint32_t original_color,
                               float percentage_factor);

#endif