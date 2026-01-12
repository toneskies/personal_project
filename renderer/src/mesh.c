#include "mesh.h"
#include "display.h"

#define N_CUBE_NODES (9 * 9 * 9)

Mesh mesh;
Node nodes[N_CUBE_NODES];

Mesh *load_generated_mesh(void) {
    mesh.nodes = nodes;
    mesh.num_nodes = N_CUBE_NODES;

    int count = 0;
    for (float x = -1; x <= 1; x += 0.25) {
        for (float y = -1; y <= 1; y += 0.25) {
            for (float z = -1; z <= 1; z += 0.25) {
                vec3_t p = {.x = x, .y = y, .z = z};
                mesh.nodes[count].position = p;
                count++;
            }
        }
    }
    return &mesh;
}