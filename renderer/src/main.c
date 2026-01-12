#include <stdio.h>
#include <stdlib.h>

#include "display.h"
#include "light.h"
#include "mesh.h"
#include "stb_ds.h"

Mesh mesh = {0};

int main(int argc, char *argv[]) {

    memset(&app, 0, sizeof(App));

    initSDL();

    setup(&mesh);

    init_light((vec3_t){0, 0, 1});

    atexit(cleanup);

    while (1) {

        // Loop
        doInput();
        update(&mesh);
        render(&mesh);
    }

    free_mesh(&mesh);

    return 0;
}