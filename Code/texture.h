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

static const char* TextureTypeStr[] = {"COLOR", "DEPTH", "STENCIL"}; 
enum class TextureType
{
    NON_FBO,
    FBO_COLOR,
    FBO_DEPTH,
    FBO_STENCIL
};

struct Texture
{
    GLuint      handle;
    std::string path;
    TextureType type = TextureType::NON_FBO;
};

struct TextureSupport
{
    static Image LoadImage(const char* filename);
    static void FreeImage(const Image& image);
    static GLuint CreateTexture2DFromImage(const Image& image);
    static u32 LoadTexture2D(App* app, const char* filepath);

    static u32 CreateEmptyColorTexture(App* app, const char* name, const u32 width, const u32 height);
    static u32 CreateEmptyDepthTexture(App* app, const char* name, const u32 width, const u32 height);

    static void ResizeTexture(App* app, const Texture& texToResize, const u32 newWidth, const u32 newHeight);
};

#endif // TEXTURE_H
