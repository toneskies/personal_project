#ifndef MESH_H
#define MESH_H

#include "triangle.h"
#include "vector.h"

typedef struct {
    vec3_t position;
} Node;

typedef struct {
    vec3_t *vertices;   // Dynamic array (stb_ds)
    face_t *faces;      // Dynamic array (stb_ds)
    vec3_t rotation;    // Per-mesh rotation
    vec3_t scale;       // Per-mesh scale
    vec3_t translation; // Per-mesh position
} Mesh;

Mesh load_mesh(char *filename);

void free_mesh(Mesh *mesh);

#endif