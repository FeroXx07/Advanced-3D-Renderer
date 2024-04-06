#ifndef APP_H
#define APP_H
#include "platform.h"
#include <vector>
#include <memory>

#include "camera.h"
#include "entity.h"
#include "light.h"
#include "mesh.h"
#include "program.h"
#include "texture.h"

static const char* RenderingModeStr[] = {"FORWARD", "DEFERRED"};
enum RenderingMode
{
    FORWARD,
    DEFERRED
};

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
    RenderingMode renderingMode = RenderingMode::FORWARD;
    
    // Camera
    Camera camera;
    glm::mat4 projectionMat;
    
    // Vectors
    std::vector<Texture>  textures;
    std::vector<Program>  programs;
    std::vector<Mesh> meshes;
    std::vector<Material> materials;
    std::vector<Model> models;
    std::vector<std::shared_ptr<Entity>> entities;
    std::vector<std::shared_ptr<Light>> lights;
    
    // texture indices
    u32 diceTexIdx;

    // Mode
    Mode mode;

    // Buffers
    Buffer uniformBuffer;
    u32 globalParamsOffset = 0;
    u32 globalParamsSize = 0;
    
    // Embedded geometry (in-editor simple meshes such as
    // a screen filling quad, a cube, a sphere...)

    // Location of the texture uniform in the textured quad shader
    GLuint defaultShaderProgram_uTexture;

    // Debug
    bool showDemoWindow = false;
    bool debugUBO = false;
};

#endif // APP_H
