//
// engine.cpp : Put all your graphics stuff in this file. This is kind of the graphics module.
// In here, you should type all your OpenGL commands, and you can also type code to handle
// input platform events (e.g to move the camera or react to certain shortcuts), writing some
// graphics related GUI options, and so on.
//

// ReSharper disable CommentTypo
// ReSharper disable CppCStyleCast
#include "engine.h"
#include <imgui.h>
#include <iostream>

#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>

#include "assimp_model_loading.h"
#include "mesh_example.h"
#include "program.h"
#include "texture.h"
#include "vertex.h"

#include "ImGuizmo.h"


void Init(App* app)
{
    // OpenGL inits
    app->ctx = RetrieveOpenGLContext();
    constexpr glm::mat4 identityMat = glm::identity<glm::mat4>();
    BufferManagement::InitUniformBuffer();

    // Create uniform buffer
    app->uniformBuffer = CREATE_CONSTANT_BUFFER(BufferManagement::maxUniformBufferSize, nullptr);

    // Create frame buffer and a color texture attachment
    app->frameBufferObject = FrameBufferManagement::CreateFrameBuffer();
    app->colorTexture = TextureSupport::CreateEmptyColorTexture(app->displaySize.x, app->displaySize.y);
    FrameBufferManagement::BindFrameBuffer(app->frameBufferObject);
    FrameBufferManagement::SetColorAttachment(app->frameBufferObject, app->colorTexture.handle, 0);
    FrameBufferManagement::CheckStatus();
    const std::vector<u32> attachments = {0};
    FrameBufferManagement::SetDrawBuffersTextures(attachments);
    FrameBufferManagement::UnBindFrameBuffer(app->frameBufferObject);
    
    // Camera & View init
    app->projectionMat = glm::perspective(glm::radians(app->camera.zoom), (float)app->displaySize.x / (float)app->displaySize.y, 0.1f, 100.0f);
    
    // Default Texture loading
    app->diceTexIdx = TextureSupport::LoadTexture2D(app, "dice.png");
    
    // Programs (and retrieve uniform indices)
    const u32 litTexturedProgramIdx = ShaderSupport::LoadProgram(app, "Shaders\\shader_lit_textured.vert", "Shaders\\shader_lit_textured.frag", "LIT_TEXTURED");
    const u32 litBaseProgramIdx = ShaderSupport::LoadProgram(app, "Shaders\\shader_lit_base.vert", "Shaders\\shader_lit_base.frag", "LIT_BASE");

    const u32 unlitBaseProgramIdx = ShaderSupport::LoadProgram(app, "Shaders\\shader_unlit_base.vert", "Shaders\\shader_unlit_base.frag", "UNLIT_BASE");
    const u32 unlitTexturedProgramIdx = ShaderSupport::LoadProgram(app, "Shaders\\shader_unlit_textured.vert", "Shaders\\shader_unlit_textured.frag", "UNLIT_TEXTURED");

    // Fill vertex shader layout auto
    for (u32 p = 0; p < app->programs.size(); ++p)
    {
        Program& program = app->programs[p];
        int programAttributesCount = 0;
        glGetProgramiv(program.handle, GL_ACTIVE_ATTRIBUTES, &programAttributesCount);
        for (int i = 0; i < programAttributesCount; ++i)
        {
            // Vertex Shader Attribute debug info
            char attributeName[64];
            i32 attributeNameLength;
            i32 attributeSize;
            u32 attributeType;
            glGetActiveAttrib(program.handle, i, std::size(attributeName), &attributeNameLength, &attributeSize, &attributeType, attributeName);
            const u32 attributeLocation = glGetAttribLocation(program.handle, attributeName);
            std::cout << "Program name: " << program.programName << ", Attribute index: " << i << ", Name: " << attributeName << ", Size: " << attributeSize <<
                ", Type: " << convertOpenGLDataTypeToString(attributeType) << ", Layout Location: " << attributeLocation << '\n';

            // Vertex Shader Attribute fill
            program.vertexInputLayout.attributes.push_back({static_cast<u8>(attributeLocation), static_cast<u8>(attributeSize)});
        }
    }
    
    app->mode = Mode_TexturedQuad;

    // Load models
    const u32 patrickModelIdx = AssimpSupport::LoadModel(app, "Patrick\\Patrick.obj");
    const u32 sampleMeshModelIdx = CreateSampleMesh(app);
    const u32 cubeModelIdx = AssimpSupport::LoadModel(app, "Primitives\\Cube.obj");
    const u32 arrowsModelIdx = AssimpSupport::LoadModel(app, "Primitives\\Arrows.obj");

    // Create entities
    CreateEntity(app, glm::vec3(-2.0f, -3.5f, 0.0f), glm::vec3(0.0f),glm::vec3(3.0f)
        ,sampleMeshModelIdx, litTexturedProgramIdx, glm::vec4(1.0f), "SampleModel");
    CreateEntity(app, glm::vec3(0.0f, 4.0f, 0.0f), glm::vec3(0.0f),glm::vec3(1.0f)
        ,cubeModelIdx, litBaseProgramIdx, glm::vec4(1.0f), "Cube");
    CreateEntity(app, glm::vec3(-12.0f, 0.0f, 0.0f), glm::vec3(0.0f),glm::vec3(1.0f)
        ,patrickModelIdx, unlitBaseProgramIdx, glm::vec4(0.788f, 0.522f, 0.02f, 1.0f), "PatrickModel");
     CreateEntity(app, glm::vec3(-6.0f, 0.0f, 0.0f), glm::vec3(0.0f),glm::vec3(1.0f)
         ,patrickModelIdx, litBaseProgramIdx, glm::vec4(0.788f, 0.522f, 0.02f, 1.0f), "PatrickModel");
     CreateEntity(app, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f),glm::vec3(1.0f)
         ,patrickModelIdx, unlitTexturedProgramIdx, glm::vec4(0.788f, 0.522f, 0.02f, 1.0f), "PatrickModel");
     CreateEntity(app, glm::vec3(6.0f, 0.0f, 0.0f), glm::vec3(0.0f),glm::vec3(1.0f)
        ,patrickModelIdx, litTexturedProgramIdx, glm::vec4(0.788f, 0.522f, 0.02f, 1.0f), "PatrickModel");
    
    // CreateLight(app, LightType::DIRECTIONAL, glm::vec3(0.0f, 3.0f, 0.0f), glm::vec3(0.0f, 45.0f, 0.0f),glm::vec3(0.2f)
     //     ,arrowsModelIdx, unlitBaseProgramIdx, glm::vec4(1.0f), "Directional Light");
    CreateLight(app, LightType::POINT, glm::vec3(0.0f, 5.0f, 0.0f), glm::vec3(0.0f),glm::vec3(0.2f)
       ,cubeModelIdx, unlitBaseProgramIdx, glm::vec4(0.955f, 1.0f, 0.5f, 1.0f), "Point Light");
    // CreateLight(app, LightType::POINT, glm::vec3(-2.0f, 2.0f, 0.0f), glm::vec3(0.0f),glm::vec3(0.2f)
    //    ,cubeModelIdx, unlitBaseProgramIdx, glm::vec4(1.0f, 0.5f, 0.0f, 1.0f), "Point Light");

    // Set camera intial pos
    app->camera.position = glm::vec3(-3.0f, 1.0f, 25.0f);
}

void CameraGUI(App* app) {
    
    // Update camera
    ImGui::Text("Camera");
    ImGui::InputFloat3("Position##Camera", &app->camera.position[0]);
    if(ImGui::InputFloat3("Pitch/Yaw/Roll", &app->camera.angles[0]))
        app->camera.UpdateCameraVectors();
}
void OpenGLContextGUI(App* app) {
    
    // Debug checkboxes
    ImGui::Checkbox("Show Demo Window", &app->showDemoWindow);
    ImGui::Checkbox("Draw Wireframe", &app->drawWireFrame);
    ImGui::Checkbox("Debug UBO", &app->debugUBO);

    // Rendering mode selection
    int renderingModeSelection = static_cast<int>(app->renderingMode);
    ImGui::Combo("Rendering Mode", &renderingModeSelection, RenderingModeStr, IM_ARRAYSIZE(RenderingModeStr));
    app->renderingMode = static_cast<RenderingMode>(renderingModeSelection);

    // Full OpenGL & GLSL info dump
    ImGui::Separator();
    ImGui::Text("OpenGL Context");
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "FPS:");
    ImGui::SameLine();
    ImGui::Text("%f", 1.0f/app->deltaTime);
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "OpenGL version:");
    ImGui::SameLine();
    ImGui::Text("%s", app->ctx.version.c_str());
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "OpenGL renderer:");
    ImGui::SameLine();
    ImGui::Text("%s", app->ctx.renderer.c_str());
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "OpenGL vendor:");
    ImGui::SameLine();
    ImGui::Text("%s", app->ctx.vendor.c_str());
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "OpenGL GLSL version:");
    ImGui::SameLine();
    ImGui::Text("%s", app->ctx.glslVersion.c_str());

    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Uniform Buffer max size:");
    ImGui::SameLine();
    ImGui::Text("%s", std::to_string(BufferManagement::maxUniformBufferSize).c_str());

    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Uniform Buffer block alignment:");
    ImGui::SameLine();
    ImGui::Text("%s", std::to_string(BufferManagement::uniformBlockAlignment).c_str());
    
}
void EntityHierarchyGUI(const App* app)
{
    if (ImGui::CollapsingHeader("Entity Hierarchy", ImGuiTreeNodeFlags_None))
    {
        const i64 entitiesCount = (i64)app->entities.size();
        for (int i = 0; i < entitiesCount; i++)
        {
            // Disable the default "open on single-click behavior" + set Selected flag according to our selection.
            ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
            const bool isSelected = selectedEntity == i;
            if (isSelected)
                nodeFlags |= ImGuiTreeNodeFlags_Selected;
            nodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen; // ImGuiTreeNodeFlags_Bullet
            ImGui::TreeNodeEx((void*)(intptr_t)i, nodeFlags, "%s %d", app->entities[i]->name.c_str(), i);  // NOLINT(performance-no-int-to-ptr)
            if (ImGui::IsItemClicked())
                selectedEntity = i;
        }
    }
}

void EntityTransformGUI(App* app)
{
    ImGui::Text("Entity: %s", app->entities[selectedEntity]->name.c_str());
    
    if (app->entities[selectedEntity])
    {
        Entity& entity = *app->entities[selectedEntity];
        EditTransform(app, glm::value_ptr(app->camera.GetViewMatrix()), glm::value_ptr(app->projectionMat), glm::value_ptr(entity.worldMatrix), true);
    }
}

void ProgramsGUI(const App* app) {
    u32& itemCurrentIdx = app->entities[selectedEntity]->programIndex;                    // Here our selection data is an index.
    const char* comboLabel = app->programs[itemCurrentIdx].programName.c_str();  // Label to preview before opening the combo (technically it could be anything)
    if (ImGui::BeginCombo("Active program", comboLabel))
    {
        for (u32 n = 0; n < app->programs.size(); n++)
        {
            const bool isSelected = (itemCurrentIdx == n);
            if (ImGui::Selectable(app->programs[n].programName.c_str(), isSelected))
                itemCurrentIdx = n;

            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
            if (isSelected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
}
void Gui(App* app)
{
    if (app->showDemoWindow)
        ImGui::ShowDemoWindow(&app->showDemoWindow);
    
    ImGui::Begin("Debug");
    OpenGLContextGUI(app);
    ImGui::Separator();
    CameraGUI(app);
    ImGui::Separator();
    EntityTransformGUI(app);
    ImGui::Separator();
    ProgramsGUI(app);
    ImGui::Separator();
    EntityHierarchyGUI(app);
    ImGui::End();
}

void Update(App* app)
{
    // Update camera inputs
    Camera& camera = app->camera;
    
    if (app->input.keys[K_W] == BUTTON_PRESSED)
        camera.ProcessKeyboard(Camera_Movement::FORWARD, app->deltaTime);
    if (app->input.keys[K_S] == BUTTON_PRESSED)
        camera.ProcessKeyboard(Camera_Movement::BACKWARD, app->deltaTime);
    if (app->input.keys[K_A] == BUTTON_PRESSED)
        camera.ProcessKeyboard(Camera_Movement::LEFT, app->deltaTime);
    if (app->input.keys[K_D] == BUTTON_PRESSED)
        camera.ProcessKeyboard(Camera_Movement::RIGHT, app->deltaTime);

    if (app->input.mouseButtons[MouseButton::RIGHT] == BUTTON_PRESSED)
        camera.ProcessMouseMovement(app->input.mouseDelta.x, -app->input.mouseDelta.y);

    // Update projection matrix after new camera inputs
    app->projectionMat = glm::perspective(glm::radians(app->camera.zoom), (float)app->displaySize.x / (float)app->displaySize.y, 0.1f, 100.0f);

    // Programs hot reload
    CheckShadersHotReload(app);

    // Uniform buffers push
    Buffer& uniformBuffer = app->uniformBuffer;
    BufferManagement::MapBuffer(uniformBuffer, GL_READ_WRITE);
    PushTransformDataToShader(app);
    PushLightDataToShader(app);
    BufferManagement::UnmapBuffer(uniformBuffer);
}

void Render(App* app)
{
    switch (app->renderingMode) {
    case FORWARD:
        ForwardRender(app);
        break;
    case DEFERRED:
        DeferredRender(app);
        break;
    }
}
  
void ForwardRender(App* app)
{
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 1, -1, "Engine Render");
    glEnable(GL_DEPTH_TEST);
    
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glViewport(0, 0, app->displaySize.x, app->displaySize.y);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    switch (app->mode)
    {
    case Mode_TexturedQuad:
        {
            // - clear the framebuffer
            // - set the viewport
            // - set the blending state
            // - bind the texture into unit 0
            // - bind the program 
            //   (...and make its texture sample from unit 0)
            // - bind the vao
            // - glDrawElements() !!!
        
            BufferManagement::BindBufferRange(app->uniformBuffer, 0, app->globalParamsSize, app->globalParamsOffset);

            const u32 entityCount = static_cast<u32>(app->entities.size());
            for (u32 e = 0; e < entityCount; ++e)
            {
                const Entity& entity = *app->entities[e];
                
                const Program& program = app->programs[entity.programIndex];
                app->defaultShaderProgram_uTexture = glGetUniformLocation(program.handle, "uTexture");
                glUseProgram(program.handle);
                BufferManagement::BindBufferRange(app->uniformBuffer, 1, entity.localParamsSize, entity.localParamsOffset);

                Model& model = app->models[entity.modelIndex];
                Mesh& mesh = app->meshes[model.meshIdx];
                
                glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 1, -1, model.name.c_str());

                const u32 subMeshCount = static_cast<u32>(mesh.subMeshes.size());
                for (u32 i = 0; i < subMeshCount; i++)
                {
                    SubMesh& subMesh = mesh.subMeshes[i];

                    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 1, -1, subMesh.name.c_str());
                    const GLuint vao = VAOSupport::FindVAO(mesh, i, program);
                
                    if (app->drawWireFrame)
                        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                    else
                        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                
                    glBindVertexArray(vao);

                    const u32 subMeshMaterialIdx = model.materialIdx[i];
                    const Material subMeshMaterial = app->materials[subMeshMaterialIdx];

                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, app->textures[subMeshMaterial.albedoTextureIdx].handle);
                    glUniform1i((GLint)app->defaultShaderProgram_uTexture, 0); // stackoverflow.com/questions/23687102/gluniform1f-vs-gluniform1i-confusion

                    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(subMesh.indices.size()), GL_UNSIGNED_INT, (void*)static_cast<u64>(subMesh.indexOffset));
                    glPopDebugGroup();
                }
                glPopDebugGroup();
            }
        }
        break;
    case Mode_Count:
        break;
    }
    glPopDebugGroup();
}
void DeferredRender(App* app) {}
void CheckShadersHotReload(App* app)
{
    for (u64 i = 0; i < app->programs.size(); ++i)
    {
        Program& program = app->programs[i];
        for (u64 j = 0; j < program.filePaths.size(); ++j)
        {
            const u64 currentTimeStamp = GetFileLastWriteTimestamp(program.filePaths[j].c_str());
            if (currentTimeStamp > program.lastWriteTimestamp)
            {
                std::cout << "Program hot reload: " << program.programName << ", TimeStamp: " << currentTimeStamp <<"\n\n";
                glDeleteProgram(program.handle);
                const char* programName = program.programName.c_str();
                if (program.filePaths.size() > 1)
                {
                    const String programSourceVert = ReadTextFile(program.filePaths[0].c_str());
                    const String programSourceFrag = ReadTextFile(program.filePaths[1].c_str());
                    program.handle = ShaderSupport::CreateProgramFromSource(programSourceVert.str, programSourceFrag.str, programName);
                }
                else
                {
                    const String programSource = ReadTextFile(program.filePaths[j].c_str());
                    program.handle = ShaderSupport::CreateProgramFromSource(programSource.str, programName);
                }
                program.lastWriteTimestamp = currentTimeStamp;
            }
        }
    }
}
void CreateEntity(App* app, const glm::vec3& position, const glm::vec3& orientation, const glm::vec3& scale, const u32 modelIndex, const u32 programIdx, const glm::vec4& diffuseColor, const char* name)
{
    std::shared_ptr<Entity> entity = std::make_shared<Entity>();
    entity->name = name;
    
    entity->position = position;
    entity->orientationEuler = orientation;
    entity->orientationQuat = glm::quat(glm::radians(orientation));
    entity->scale = scale;
    
    entity->worldMatrix = glm::mat4(1.0f);
    entity->worldMatrix = glm::translate(entity->worldMatrix, entity->position);
    entity->worldMatrix *= glm::toMat4(entity->orientationQuat);
    entity->worldMatrix = glm::scale(entity->worldMatrix, entity->scale);
    
    entity->modelIndex = modelIndex;
    entity->color = diffuseColor;
    entity->programIndex = programIdx;
    
    app->entities.emplace_back(entity);
}
void CreateLight(App* app, LightType lightType, const glm::vec3& position, const glm::vec3& orientation, const glm::vec3& scale, const u32 modelIndex, const u32 programIdx, const glm::vec4& lightColor, const char* name)
{
    std::shared_ptr<Light> light = std::make_shared<Light>();
    light->type = lightType;
    light->eType = EntityType::LIGHT;
    
    light->name = name;
    
    light->position = position;
    light->orientationEuler = orientation;
    light->orientationQuat = glm::quat(glm::radians(orientation));
    light->scale = scale;
    
    light->worldMatrix = glm::mat4(1.0f);
    light->worldMatrix = glm::translate(light->worldMatrix, light->position);
    light->worldMatrix *= glm::toMat4(light->orientationQuat);
    light->worldMatrix = glm::scale(light->worldMatrix, light->scale);

    light->modelIndex = modelIndex;
    light->color = lightColor;
    light->programIndex = programIdx;
    
    app->entities.emplace_back(light);
    app->lights.emplace_back(light);
}

void PushTransformDataToShader(App* app)
{
    Buffer& uniformBuffer = app->uniformBuffer;
    
    const u64 entityCount = app->entities.size();
    for (u32 i = 0; i < entityCount; ++i)
    {
        Entity& entity = *app->entities[i];

        // Calculate MVP and Normal Matrix for the lightning
        glm::mat4 ModelViewMat = app->camera.GetViewMatrix() * entity.worldMatrix;
        entity.worldViewProjectionMat = app->projectionMat * ModelViewMat;
        entity.normalMatrix = glm::mat3(glm::transpose(glm::inverse(ModelViewMat)));

        // Set buffer block start and set offset
        BufferManagement::SetBufferBlockStart(uniformBuffer, BufferManagement::uniformBlockAlignment, entity.localParamsOffset);

        PUSH_VEC4(uniformBuffer, entity.color);
        PUSH_MAT4(uniformBuffer, entity.worldMatrix);
        PUSH_MAT4(uniformBuffer, entity.worldViewProjectionMat);
        PUSH_MAT4(uniformBuffer, entity.normalMatrix);

        // Set buffer block end and set size
        BufferManagement::SetBufferBlockEnd(uniformBuffer, BufferManagement::uniformBlockAlignment, entity.localParamsSize, entity.localParamsOffset);

        if (app->debugUBO)
            std::cout << "Entity: " << entity.name.c_str() << ". Local Params Offset: " << entity.localParamsOffset << ". Local Params Size: " << entity.localParamsSize  << "\n";
    }
    
    if (app->debugUBO)
        std::cout << "\n";
}
void PushLightDataToShader(App* app)
{
    Buffer& uniformBuffer = app->uniformBuffer;

    // Set buffer block start and set offset
    BufferManagement::SetBufferBlockStart(uniformBuffer, BufferManagement::uniformBlockAlignment, app->globalParamsOffset);
    
    const u32 lightsCount = (u32)app->lights.size();
    PUSH_VEC3(uniformBuffer, app->camera.position);
    PUSH_U_INT(uniformBuffer, lightsCount)
    for (u32 i = 0; i < lightsCount; ++i)
    {
        // Correct if necessary the alignment of array 
        BufferManagement::AlignHead(uniformBuffer, 4 * BASIC_MACHINE_UNIT);

        const Light& light = *app->lights[i];
        PUSH_U_INT(uniformBuffer, (u32)light.type)
        PUSH_VEC3(uniformBuffer, light.color);
        PUSH_VEC3(uniformBuffer, light.orientationEuler);
        PUSH_VEC3(uniformBuffer, light.position);
    }

    // Set buffer block end and set size
    BufferManagement::SetBufferBlockEnd(uniformBuffer, BufferManagement::uniformBlockAlignment, app->globalParamsSize, app->globalParamsOffset);

    if (app->debugUBO)
        std::cout << "Global Params. " << " Offset: " << app->globalParamsOffset << " Size: " << app->globalParamsSize  << "\n";
}


void EditTransform(App* app, const float* cameraView, float* cameraProjection, float* matrix, bool editTransformDecomposition)
{
    if (editTransformDecomposition)
    {
        // Keyboard shortcuts for input mode
        if (app->input.keys[K_1] == BUTTON_PRESS)
            app->imGuizmoData.mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
        if (app->input.keys[K_2] == BUTTON_PRESS)
            app->imGuizmoData.mCurrentGizmoOperation = ImGuizmo::ROTATE;
        if (app->input.keys[K_3] == BUTTON_PRESS) 
            app->imGuizmoData.mCurrentGizmoOperation = ImGuizmo::SCALE;

        // Mouse buttons for input mode
        if (ImGui::RadioButton("Translate", app->imGuizmoData.mCurrentGizmoOperation == ImGuizmo::TRANSLATE))
            app->imGuizmoData.mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
        ImGui::SameLine();
        if (ImGui::RadioButton("Rotate", app->imGuizmoData.mCurrentGizmoOperation == ImGuizmo::ROTATE))
            app->imGuizmoData.mCurrentGizmoOperation = ImGuizmo::ROTATE;
        ImGui::SameLine();
        if (ImGui::RadioButton("Scale", app->imGuizmoData.mCurrentGizmoOperation == ImGuizmo::SCALE))
            app->imGuizmoData.mCurrentGizmoOperation = ImGuizmo::SCALE;

        // Inputs for transform
        float matrixTranslation[3], matrixRotation[3], matrixScale[3];
        ImGuizmo::DecomposeMatrixToComponents(matrix, matrixTranslation, matrixRotation, matrixScale);
        ImGui::InputFloat3("Translate", matrixTranslation);
        ImGui::InputFloat3("Rotate", matrixRotation);
        ImGui::InputFloat3("Scale", matrixScale);
        ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, matrix);

        // World/local options
        if (app->imGuizmoData.mCurrentGizmoOperation != ImGuizmo::SCALE)
        {
            if (ImGui::RadioButton("Local", app->imGuizmoData.mCurrentGizmoMode == ImGuizmo::LOCAL))
                app->imGuizmoData.mCurrentGizmoMode = ImGuizmo::LOCAL;
            ImGui::SameLine();
            if (ImGui::RadioButton("World", app->imGuizmoData.mCurrentGizmoMode == ImGuizmo::WORLD))
                app->imGuizmoData.mCurrentGizmoMode = ImGuizmo::WORLD;
        }

        // Key shortcut for toggling snap
        if (app->input.keys[K_4] == BUTTON_PRESS)
            app->imGuizmoData.useSnap = !app->imGuizmoData.useSnap;
        ImGui::Checkbox("Use Snap", &app->imGuizmoData.useSnap);
        ImGui::SameLine();

        // Snaping options
        switch (app->imGuizmoData.mCurrentGizmoOperation)
        {
        case ImGuizmo::TRANSLATE:
            ImGui::InputFloat3("Snap", &app->imGuizmoData.snap[0]);
            break;
        case ImGuizmo::ROTATE:
            ImGui::InputFloat("Angle Snap", &app->imGuizmoData.snap[0]);
            break;
        case ImGuizmo::SCALE:
            ImGui::InputFloat("Scale Snap", &app->imGuizmoData.snap[0]);
            break;
        default: ;
        }

        // Bound sizing inputs
        ImGui::Checkbox("Bound Sizing", &app->imGuizmoData.boundSizing);
        if (app->imGuizmoData.boundSizing)
        {
            ImGui::PushID(3);
            ImGui::Checkbox("Bound Sizing Snap", &app->imGuizmoData.boundSizingSnap);
            ImGui::SameLine();
            ImGui::InputFloat3("Snap", app->imGuizmoData.boundsSnap);
            ImGui::PopID();
        }
    }
    
    // Imguizmo for transform
    const ImGuiIO& io = ImGui::GetIO();
    float viewManipulateRight = io.DisplaySize.x;
    float viewManipulateTop = 0;
    ImGuizmo::SetRect((float)app->displayPos.x, (float)app->displayPos.y, (float)app->displaySize.x, (float)app->displaySize.y);
    //ImGuizmo::DrawGrid(cameraView, glm::value_ptr(app->projectionMat), glm::value_ptr(glm::mat4(1.0f)), 100.f);
    ImGuizmo::Manipulate(cameraView, glm::value_ptr(app->projectionMat), app->imGuizmoData.mCurrentGizmoOperation, app->imGuizmoData.mCurrentGizmoMode, glm::value_ptr(app->entities[selectedEntity]->worldMatrix), nullptr, app->imGuizmoData.useSnap ? &app->imGuizmoData.snap[0] : nullptr, app->imGuizmoData.boundSizing ? app->imGuizmoData.bounds : nullptr, app->imGuizmoData.boundSizingSnap ? app->imGuizmoData.boundsSnap : nullptr);

    //ImGuizmo::ViewManipulate(cameraView, camDistance, ImVec2(viewManipulateRight - 128, viewManipulateTop), ImVec2(128, 128), 0x10101010);
}

void VAOSupport::CreateNewVAO(const Mesh& mesh, const SubMesh& subMesh, const Program& program, GLuint& vaoHandle)
{
    std::cout << "Creating new VAO for Mesh: " << mesh.name << " SubMesh: " << subMesh.name << " With program " << program.programName << "\n" << "\n";
    glGenVertexArrays(1, &vaoHandle);
    glBindVertexArray(vaoHandle);
    BufferManagement::BindBuffer(mesh.vertexBuffer);
    BufferManagement::BindBuffer(mesh.indexBuffer);
    
    //glBindBuffer(GL_ARRAY_BUFFER, mesh.vertexBuffer);
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.indexBuffer);

    // We have to link all vertex inputs attributes to attributes in the vertex buffer
    const u32 programAttributesCount = (u32)program.vertexInputLayout.attributes.size();
    for (u32 i = 0; i < programAttributesCount; ++i)
    {
        bool attributeWasLinked = false;
        const u32 subMeshAttributesCount = (u32)subMesh.vertexBufferLayout.attributes.size();
        for(u32 j = 0; j < subMeshAttributesCount; ++j)
        {
            if (program.vertexInputLayout.attributes[i].location == subMesh.vertexBufferLayout.attributes[j].location)
            {
                const u32 index = subMesh.vertexBufferLayout.attributes[j].location;
                const u32 nComp = subMesh.vertexBufferLayout.attributes[j].componentCount;
                // Since it shares the same buffers in the mesh it will use its corresponding array rang with vertex offset here, and indices offset when glDrawElements
                const u32 offset = subMesh.vertexBufferLayout.attributes[j].offset + subMesh.vertexOffset; // attribute offset + vertex offset
                const u32 stride = subMesh.vertexBufferLayout.stride;
                glVertexAttribPointer(index, (GLsizei)nComp, GL_FLOAT, GL_FALSE, (GLsizei)stride, (void*)(u64)offset);
                glEnableVertexAttribArray(index);

                attributeWasLinked = true;
                break;
            }
        }
        assert(attributeWasLinked); // The subMesh should provide an attribute for each vertex inputs
    }
    glBindVertexArray(0);
}

GLuint VAOSupport::FindVAO(Mesh& mesh, const u32 subMeshIndex, const Program& program)
{
    SubMesh& subMesh = mesh.subMeshes[subMeshIndex];

    // Try finding VAO for this subMesh/program
    const u32 vaoListSize = (u32)subMesh.vaoList.size();
    for (u32 i = 0; i < vaoListSize; ++i)
    {
        if (subMesh.vaoList[i].programHandle == program.handle)
            return subMesh.vaoList[i].handle;
    }
    
    GLuint vaoHandle = 0;

    // Create a new vao for this subMesh/program
    CreateNewVAO(mesh, subMesh, program, vaoHandle);

    // Store it in the list of VAOs for this subMesh
    const VAO vao  = {vaoHandle, program.handle};
    subMesh.vaoList.push_back(vao);

    return  vaoHandle;
}
