#ifndef TEXTURE_H
#define TEXTURE_H

#include <GL/glew.h>

#include <GL/gl.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "core/file.h"
#include "core/mem.h"

struct texture {
    GLuint id;
    int width;
    int height;
};

typedef struct texture texture;

texture *texture_make(char *path, GLint clamp, GLint interpolate);

#endif