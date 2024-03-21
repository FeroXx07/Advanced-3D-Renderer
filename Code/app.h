#ifndef APP_H
#define APP_H
#include "platform.h"
#include <vector>

#include "mesh.h"
#include "program.h"
#include "texture.h"

enum Mode
{
    Mode_TexturedQuad,
    Mode_Count
};

struct App
{
    // Loop
    f32  deltaTime;
    bool isRunning;

    // Input
    Input input;

    // Graphics
    OpenGlContext ctx;
    ivec2 displaySize;
    bool drawWireFrame = false;
    
    // Vectors
    std::vector<Texture>  textures;
    std::vector<Program>  programs;
    std::vector<Mesh> meshes;
    std::vector<Material> materials;
    std::vector<Model> models;

    // Model
    u32 model = 0;
    
    // program indices
    u32 texturedGeometryProgramIdx;
    
    // texture indices
    u32 diceTexIdx;
    u32 whiteTexIdx;
    u32 blackTexIdx;
    u32 normalTexIdx;
    u32 magentaTexIdx;

    // Mode
    Mode mode;

    // Embedded geometry (in-editor simple meshes such as
    // a screen filling quad, a cube, a sphere...)

    // Location of the texture uniform in the textured quad shader
    GLuint defaultShaderProgram_uTexture;

    bool showDemoWindow = false;
};

#endif // APP_H
