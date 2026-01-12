#include <stdio.h>
#include <stdlib.h>

#include "display.h"
#include "mesh.h"

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

int main(int argc, char *argv[]) {

    memset(&app, 0, sizeof(App));

    initSDL();

    setup();

    atexit(cleanup);

    while (1) {

        // Loop
        doInput();
        update();
        render();
    }

    return 0;
}