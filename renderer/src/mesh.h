#ifndef MESH_H
#define MESH_H

#include "vector.h"

typedef struct {
    vec3_t position;
} Node;

typedef struct {
    Node *nodes;
    int num_nodes;
} Mesh;

Mesh *load_generated_mesh(void);

#endif