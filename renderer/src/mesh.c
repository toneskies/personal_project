#include "mesh.h"
#include "display.h"
#include "stb_ds.h"
#include "triangle.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Mesh load_mesh(char *filename) {
    Mesh mesh = {0};
    mesh.vertices = NULL; // Important: Initialize stb_ds arrays to NULL
    mesh.faces = NULL;
    mesh.rotation = (vec3_t){0, 0, 0};
    mesh.scale = (vec3_t){1, 1, 1};
    mesh.translation = (vec3_t){0, 0, 0};

    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Error: Could not open file %s\n", filename);
        return mesh;
    }

    char line[1024];
    while (fgets(line, 1024, file)) {
        // 1. Vertex Information (v x y z)
        if (strncmp(line, "v ", 2) == 0) {
            vec3_t v;
            sscanf(line, "v %f %f %f", &v.x, &v.y, &v.z);
            arrput(mesh.vertices, v);
        }
        // 2. Face Information (f v1/vt/vn v2/vt/vn v3/vt/vn)
        else if (strncmp(line, "f ", 2) == 0) {
            int vertex_indices[3];
            int texture_indices[3];
            int normal_indices[3];

            // Try reading: f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3
            int matches = sscanf(
                line, "f %d/%d/%d %d/%d/%d %d/%d/%d", &vertex_indices[0],
                &texture_indices[0], &normal_indices[0], &vertex_indices[1],
                &texture_indices[1], &normal_indices[1], &vertex_indices[2],
                &texture_indices[2], &normal_indices[2]);

            // If that fails, try reading: f v1 v2 v3 (Raw geometry)
            if (matches != 9) {
                sscanf(line, "f %d %d %d", &vertex_indices[0],
                       &vertex_indices[1], &vertex_indices[2]);
            }

            // Create the face
            // OBJ indices are 1-based, so we subtract 1 for C arrays
            face_t face = {.a = vertex_indices[0] - 1,
                           .b = vertex_indices[1] - 1,
                           .c = vertex_indices[2] - 1,
                           .color = 0xFFFFFFFF};

            arrput(mesh.faces, face);
        }
    }

    fclose(file);
    return mesh;
}

void free_mesh(Mesh *mesh) {
    arrfree(mesh->vertices);
    arrfree(mesh->faces);
}
