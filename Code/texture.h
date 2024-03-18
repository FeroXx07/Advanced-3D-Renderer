#ifndef TEXTURE_H
#define TEXTURE_H
#include "platform.h"

struct Image
{
    void* pixels;
    ivec2 size;
    i32   nchannels;
    i32   stride;
};

struct Texture
{
    GLuint      handle;
    std::string filepath;
};

#endif // TEXTURE_H
