#ifndef TEXTURE_H
#define TEXTURE_H
#include "platform.h"

struct App;
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
    std::string path;
};

struct TextureSupport
{
    static Image LoadImage(const char* filename);
    static void FreeImage(const Image& image);
    static GLuint CreateTexture2DFromImage(const Image& image);
    static u32 LoadTexture2D(App* app, const char* filepath);

    static Texture CreateEmptyColorTexture(const u32 width, const u32 height);
    static Texture CreateEmptyDepthTexture(const u32 width, const u32 height);
};

#endif // TEXTURE_H
