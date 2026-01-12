#ifndef MATRIX_H
#define MATRIX_H

#include "vector.h"

typedef struct {
    float m[4][4];
} mat4_t;

// Initialize a perspective projection matrix
mat4_t mat4_make_perspective(float fov, float aspect, float znear, float zfar);

// Multiply a matrix by a vector
vec4_t mat4_mul_vec4(mat4_t m, vec4_t v);

#endif