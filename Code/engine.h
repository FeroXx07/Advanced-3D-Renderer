//
// engine.h: This file contains the types and functions relative to the engine.
//

#pragma once

#include "app.h"

void Init(App* app);

void Gui(App* app);

void Update(App* app);

void Render(App* app);

void CheckShadersHotReload(App* app);

void CreateEntity(App* app, const glm::vec3& position, const glm::vec3& orientation, const glm::vec3& scale,
    const u32 modelIndex, const u32 programIdx = 0, const glm::vec4& diffuseColor = glm::vec4(1.0f), const char* name = "None");

void CreateLight(App* app, LightType lightType, const glm::vec3& position, const glm::vec3& orientation, const glm::vec3& scale,
    const u32 modelIndex, const u32 programIdx = 0, const glm::vec4& lightColor = glm::vec4(1.0f), const char* name = "None");

void PushTransformDataToShader(App* app);

void PushLightDataToShader(App* app);

struct VAOSupport
{
    static void CreateNewVAO(const Mesh& mesh, const SubMesh& subMesh, const Program& program, GLuint& vaoHandle);
    static GLuint FindVAO(Mesh& mesh, const u32 subMeshIndex, const Program& program);
};

static i32 selectedEntity = 0;



