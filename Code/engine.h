//
// engine.h: This file contains the types and functions relative to the engine.
//

#pragma once

#include "app.h"

void Init(App* app);

void Gui(App* app);

void Update(App* app);

void Render(App* app);

void ForwardRender(App* app);

void ForwardRenderLightBoxes(App* app);

void DeferredRender(App* app);

void DeferredRenderGeometryPass(App* app);

void DeferredRenderShadingPass(App* app);

void DeferredRenderDisplayPass(App* app);

void CheckShadersHotReload(App* app);

void CreateEntity(App* app, const glm::vec3& position, const glm::vec3& orientation, const glm::vec3& scale,
    const u32 modelIndex, const u32 programIdx = 0, const glm::vec4& diffuseColor = glm::vec4(1.0f), const char* name = "None");

void CreateLight(App* app, LightType lightType, const Attenuation& attenuation, const glm::vec3& position, const glm::vec3& orientation, const glm::vec3& scale,
    const u32 modelIndex, const u32 programIdx = 0, const glm::vec4& lightColor = glm::vec4(1.0f), const char* name = "None");

void PushTransformUBO(App* app);

void PushLightDataUBO(App* app);

void PushMaterialDataUBO(App* app);

void OnScreenResize(App* app);

static i32 selectedEntity = 0;
void EditTransform(App* app, const float* cameraView, float* cameraProjection, float* matrix, bool editTransformDecomposition);


