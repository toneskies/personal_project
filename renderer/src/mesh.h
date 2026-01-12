#ifndef MESH_H
#define MESH_H

#include "triangle.h"
#include "vector.h"

#define N_MESH_NODES 8
#define N_MESH_FACES (6 * 2)

extern vec3_t mesh_vertices[N_MESH_NODES];
extern face_t mesh_faces[N_MESH_FACES];

typedef struct {
    vec3_t position;
} Node;

typedef struct {
    Node *nodes;
    face_t *faces;
    int num_nodes;
} Mesh;

#endif