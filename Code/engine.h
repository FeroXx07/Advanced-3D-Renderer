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

void CreateEntity(App* app, const glm::mat4& worldMatrix, const u32 modelIndex, const char* name);

void PushTransformDataToShader(App* app);

struct VAOSupport
{
    static void CreateNewVAO(const Mesh& mesh, const SubMesh& subMesh, const Program& program, GLuint& vaoHandle);
    static GLuint FindVAO(Mesh& mesh, const u32 subMeshIndex, const Program& program);
};


