#include "texture.h"

#include <glm/gtx/string_cast.hpp>

#include "app.h"
#include "stb_image.h"

Image TextureSupport::LoadImage(const char* filename)
{
    Image img = {};
    stbi_set_flip_vertically_on_load(true);
    img.pixels = stbi_load(filename, &img.size.x, &img.size.y, &img.nchannels, 0);
    if (img.pixels)
    {
        img.stride = img.size.x * img.nchannels;
    }
    else
    {
        ELOG("Could not open file %s", filename)
    }
    return img;
}

void TextureSupport::FreeImage(const Image& image)
{
    stbi_image_free(image.pixels);
}

GLuint TextureSupport::CreateTexture2DFromImage(const Image& image)
{
    GLenum internalFormat = GL_RGB8;
    GLenum dataFormat = GL_RGB;
    constexpr GLenum dataType = GL_UNSIGNED_BYTE;

    switch (image.nchannels)
    {
    case 1: dataFormat = GL_RED; internalFormat = GL_R8; break;
    case 3: dataFormat = GL_RGB; internalFormat = GL_RGB8; break;
    case 4: dataFormat = GL_RGBA; internalFormat = GL_RGBA8; break;
    default: ELOG("LoadTexture2D() - Unsupported number of channels")
    }

    GLuint texHandle;
    glGenTextures(1, &texHandle);
    glBindTexture(GL_TEXTURE_2D, texHandle);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, image.size.x, image.size.y, 0, dataFormat, dataType, image.pixels);
    //https://www.khronos.org/opengl/wiki/Common_Mistakes#:~:text=requires%20GL%203.0).-,Checking%20for%20OpenGL%20Errors,-%5Bedit%5D
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    return texHandle;
}

u32 TextureSupport::LoadTexture2D(App* app, const char* filepath)
{
    for (u32 texIdx = 0; texIdx < app->textures.size(); ++texIdx)
        if (app->textures[texIdx].path == filepath)
            return texIdx;

    Image image = LoadImage(filepath);

    if (image.pixels)
    {
        Texture tex = {};
        tex.handle = CreateTexture2DFromImage(image);
        tex.path = filepath;
        tex.size = image.size;
        
        const u32 texIdx = static_cast<u32>(app->textures.size());
        app->textures.push_back(tex);

        FreeImage(image);
        return texIdx;
    }
    else
    {
        //return UINT32_MAX;
        return app->defaultTextureIdx;
    }
}

u32 TextureSupport::CreateEmptyColorTexture_8Bit_RGBA(App* app, const char* name, const u32 width, const u32 height)
{
    Texture tex = {};
    tex.path = name;
    tex.type = TextureType::FBO_COLOR_8_BIT_RGBA;
    tex.size.x = static_cast<i32>(width);
    tex.size.y = static_cast<i32>(height);
    
    glGenTextures(1, &tex.handle);
    glBindTexture(GL_TEXTURE_2D, tex.handle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, static_cast<GLsizei>(width), static_cast<GLsizei>(height), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    const u32 texIdx = static_cast<u32>(app->textures.size());
    app->textures.push_back(tex);

    return texIdx;
}

u32 TextureSupport::CreateEmptyColorTexture_16Bit_F_RGBA(App* app, const char* name, const u32 width, const u32 height)
{
    Texture tex = {};
    tex.path = name;
    tex.type = TextureType::FBO_COLOR_16_BIT_FLOAT_RGBA;
    tex.size.x = static_cast<i32>(width);
    tex.size.y = static_cast<i32>(height);

    glGenTextures(1, &tex.handle);
    glBindTexture(GL_TEXTURE_2D, tex.handle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, static_cast<GLsizei>(width), static_cast<GLsizei>(height), 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    const u32 texIdx = static_cast<u32>(app->textures.size());
    app->textures.push_back(tex);

    return texIdx;
}

u32 TextureSupport::CreateEmptyDepthTexture(App* app, const char* name, const u32 width, const u32 height)
{
    Texture tex = {};
    tex.path = name;
    tex.type = TextureType::FBO_DEPTH;
    tex.size.x = static_cast<i32>(width);
    tex.size.y = static_cast<i32>(height);
    
    glGenTextures(1, &tex.handle);
    glBindTexture(GL_TEXTURE_2D, tex.handle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, static_cast<GLsizei>(width), static_cast<GLsizei>(height), 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    const u32 texIdx = static_cast<u32>(app->textures.size());
    app->textures.push_back(tex);

    return texIdx;
}
u32 TextureSupport::CreateEmptyColorTexture_8Bit_R(App* app, const char* name, const u32 width, const u32 height)
{
    Texture tex = {};
    tex.path = name;
    tex.type = TextureType::FBO_COLOR_8_BIT_RED;
    tex.size.x = static_cast<i32>(width);
    tex.size.y = static_cast<i32>(height);
    
    glGenTextures(1, &tex.handle);
    glBindTexture(GL_TEXTURE_2D, tex.handle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, static_cast<GLsizei>(width), static_cast<GLsizei>(height), 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    const u32 texIdx = static_cast<u32>(app->textures.size());
    app->textures.push_back(tex);

    return texIdx;
}
u32 TextureSupport::CreateNoiseColorTexture_16Bit_F_RGBA(App* app, const char* name, const u32 width, const u32 height, const std::vector<glm::vec3>& ssaoNoise)
{
    Texture tex = {};
    tex.path = name;
    tex.type = TextureType::NON_FBO;
    tex.size.x = static_cast<i32>(width);
    tex.size.y = static_cast<i32>(height);

    glGenTextures(1, &tex.handle);
    glBindTexture(GL_TEXTURE_2D, tex.handle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, static_cast<GLsizei>(width), static_cast<GLsizei>(height), 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    const u32 texIdx = static_cast<u32>(app->textures.size());
    app->textures.push_back(tex);

    return texIdx;
}
void TextureSupport::ResizeTexture(App* app, Texture& texToResize, const u32 newWidth, const u32 newHeight)
{
    switch (texToResize.type) {
        case TextureType::FBO_COLOR_8_BIT_RGBA:
        {
            glBindTexture(GL_TEXTURE_2D, texToResize.handle);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, static_cast<GLsizei>(newWidth), static_cast<GLsizei>(newHeight), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); 
        }
        break;
        case TextureType::FBO_COLOR_8_BIT_RED:
        {
            glBindTexture(GL_TEXTURE_2D, texToResize.handle);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, static_cast<GLsizei>(newWidth), static_cast<GLsizei>(newHeight), 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); 
        }
        break;
    case TextureType::FBO_COLOR_16_BIT_FLOAT_RGBA:
        {
            glBindTexture(GL_TEXTURE_2D, texToResize.handle);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, static_cast<GLsizei>(newWidth), static_cast<GLsizei>(newHeight), 0, GL_RGBA, GL_FLOAT, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); 
        }
        break;
    case TextureType::FBO_DEPTH:
        {
            glBindTexture(GL_TEXTURE_2D, texToResize.handle);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, static_cast<GLsizei>(newWidth), static_cast<GLsizei>(newHeight), 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); 
        }
        break;
    case TextureType::FBO_STENCIL:
        break;
    default: ;
    }
    texToResize.size.x = static_cast<i32>(newWidth);
    texToResize.size.y = static_cast<i32>(newHeight);
    glBindTexture(GL_TEXTURE_2D, 0);
}
std::string TextureSupport::GetInfoString(const Texture& tex)
{
    std::string info = "Size: ";
    info += glm::to_string(tex.size);
    info += ", ";
    info += "Mega bytes size: ";
    float sizeBytes = 0;
    if (tex.type != TextureType::FBO_DEPTH)
    {
        sizeBytes = static_cast<float>(tex.size.x) * static_cast<float>(tex.size.y) * 4.0f/1000000.0f; // 4 bits per channel = 1 byte per channel (4 channels) from bytes to megabytes
    }
    else
    {
        sizeBytes = static_cast<float>(tex.size.x) * static_cast<float>(tex.size.y) * 3.0f/1000000.0f; // 24 bits per channel, single channel, conversion from bytes to megabytes
    }
    info += std::to_string(sizeBytes);
    return info;
}
