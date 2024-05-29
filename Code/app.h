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
#include "ImGuizmo.h"

static const char* RenderingModeStr[] = {"FORWARD", "DEFERRED"};

enum RenderingMode
{
    FORWARD,
    DEFERRED
};

static const char* GBufferModeStr[] = { "COLOR", "NORMAL", "POSITION", "SPECULAR", "DEPTH", "FINAL", "REFLECTION",
"REFLECTION_DEPTH", "REFRACTION", "REFRACTION_DEPTH"};

enum GBufferMode
{
    COLOR,
    NORMAL,
    POSITION,
    SPECULAR,
    DEPTH,
    FINAL,
    REFLECTION,
    REFLECTION_DEPTH,
    REFRACTION,
    REFRACTION_DEPTH
};

struct ImGuizmoData 
{
    bool useSnap = false;
    float snap[3] = { 1.f, 1.f, 1.f };
    float bounds[6] = { -0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f };
    float boundsSnap[3] = { 0.1f, 0.1f, 0.1f };
    bool boundSizing = false;
    bool boundSizingSnap = false;

    ImGuizmo::OPERATION mCurrentGizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
    ImGuizmo::MODE mCurrentGizmoMode = ImGuizmo::MODE::LOCAL;
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
    ivec2 displaySizeCurrent;
    ivec2 displaySizePrevious;
    ivec2 displayPos;

    // Screen quad
    u32 quadModel;
    u32 screenDisplayProgramIdx;

    // G Buffer
    Buffer frameBufferObject;
    u32 gColorTextureIdx;
    u32 gDepthTextureIdx;
    u32 gNormalTextureIdx;
    u32 gPositionTextureIdx;
    u32 gSpecularTextureIdx;
    u32 gReflectionTextureIdx;
    u32 gReflectionDepthTextureIdx;
    u32 gRefractionTextureIdx;
    u32 gRefractionDepthTextureIdx;
    u32 gFinalResultTextureIdx;
    
    bool drawWireFrame = false;
    RenderingMode renderingMode = RenderingMode::DEFERRED;
    GBufferMode gBufferMode = GBufferMode::FINAL;

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
    
    // default
    u32 defaultTextureIdx;
    u32 deferredGeometryProgramIdx;
    u32 deferredShadingProgramIdx;

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

    // ImGuizmo
    ImGuizmoData imGuizmoData;
};

#endif // APP_H
