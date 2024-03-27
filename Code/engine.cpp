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


void Init(App* app)
{
    // OpenGL inits
    app->ctx = RetrieveOpenGLContext();
    constexpr glm::mat4 identityMat = glm::identity<glm::mat4>();
    BufferManagement::InitUniformBuffer();
    app->uniformBuffer = CREATE_CONSTANT_BUFFER(BufferManagement::maxUniformBufferSize, nullptr);
    
    // Camera & View init
    app->projectionMat = glm::perspective(glm::radians(app->camera.zoom), (float)app->displaySize.x / (float)app->displaySize.y, 0.1f, 100.0f);
    
    // Default Texture loading
    app->diceTexIdx = TextureSupport::LoadTexture2D(app, "dice.png");
    
    // - programs (and retrieve uniform indices)
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

    const u32 patrickModelIdx = AssimpSupport::LoadModel(app, "Patrick\\patrick.obj");
    const u32 sampleMeshModelIdx = CreateSampleMesh(app);

    CreateEntity(app, glm::translate(identityMat, glm::vec3(0.0f, -1.0f, 0.0f)), sampleMeshModelIdx, "SampleModel", glm::vec4(1.0f), litTexturedProgramIdx);
    CreateEntity(app, glm::translate(identityMat, glm::vec3(0.0f, 0.5f, 0.0f)), AssimpSupport::LoadModel(app, "Primitives\\Cube.obj"), "Cube", glm::vec4(1.0f), litTexturedProgramIdx);

    CreateEntity(app, glm::scale(identityMat, glm::vec3(0.3f)), patrickModelIdx, "PatrickModel", glm::vec4(1.0f), litTexturedProgramIdx);
    CreateEntity(app, glm::scale(glm::translate(identityMat, glm::vec3(2.0f, 0.0f, 0.0f)), glm::vec3(0.3f)), patrickModelIdx, "PatrickModel2", glm::vec4(1.0f), litBaseProgramIdx);
    CreateEntity(app, glm::scale(glm::translate(identityMat, glm::vec3(-2.0f, 0.0f, 0.0f)), glm::vec3(0.3f)), patrickModelIdx, "PatrickModel3", glm::vec4(1.0f), unlitBaseProgramIdx);

    CreateLight(app, glm::vec3(0.0f, 0.0f, 0.0f), LightType::DIRECTIONAL, glm::vec3(0.0f), glm::vec4(0.0f, 255.0f/255.0f, 0.0f, 255.0f/255.0f));
    CreateLight(app, glm::vec3(0.0f, 2.0f, 0.0f), LightType::POINT, glm::vec3(0.0f), glm::vec4(255.0f/255.0f, 0.0f, 0.0f, 255.0f/255.0f));
   
    PushTransformDataToShader(app);

    app->camera.position = glm::vec3(0.0f, 1.0f, 8.0f);
}

void CameraGUI(App* app) {
    ImGui::Text("Camera");
    ImGui::InputFloat3("Position##Camera", &app->camera.position[0]);
    if(ImGui::InputFloat3("Pitch/Yaw/Roll", &app->camera.angles[0]))
        app->camera.UpdateCameraVectors();
}
void OpenGLContextGUI(App* app) {
    ImGui::Checkbox("#Show Demo Window", &app->showDemoWindow);
    ImGui::Checkbox("#Draw Wireframe", &app->drawWireFrame);
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
            ImGui::TreeNodeEx((void*)(intptr_t)i, nodeFlags, "%s %d", app->entities[i].name.c_str(), i);
            if (ImGui::IsItemClicked())
                selectedEntity = i;
        }
    }
}
void EntityTransformGUI(App* app) {
    ImGui::Text("Model Transform: %s", app->entities[selectedEntity].name.c_str());

    // Decompose model matrix
    glm::vec3 translation, scale, skew;
    glm::vec4 perspective;
    glm::quat orientation;
    glm::decompose(app->entities[selectedEntity].worldMatrix, scale, orientation, translation, skew, perspective);
    glm::vec3 eulerAnglesOrientation = glm::degrees(glm::eulerAngles(orientation));

    // Modify values
    bool valueChanged = false;
    if (ImGui::InputFloat3("Translation", &translation[0]) || ImGui::InputFloat3("Orientation", &eulerAnglesOrientation[0]) || ImGui::InputFloat3("Scale", &scale[0]))
        valueChanged = true;

    if (valueChanged)
    {
        // Compose model matrix
        orientation = glm::quat(glm::radians(eulerAnglesOrientation));
        scale = glm::clamp(scale, glm::vec3(0.001f), glm::vec3(100.0f));
        glm::mat4 composedModelMat = glm::mat4(1.0f);
        composedModelMat = glm::translate(composedModelMat, translation);
        composedModelMat *= glm::toMat4(orientation); // Combine rotation
        composedModelMat = glm::scale(composedModelMat, scale);
    
        app->entities[selectedEntity].worldMatrix = composedModelMat;
    }

    constexpr bool alphaPreview = true;
    constexpr bool alphaHalfPreview = false;
    constexpr bool optionsMenu = true;
    constexpr ImGuiColorEditFlags miscFlags = (alphaHalfPreview ? ImGuiColorEditFlags_AlphaPreviewHalf : (alphaPreview ? ImGuiColorEditFlags_AlphaPreview : 0)) | (optionsMenu ? 0 : ImGuiColorEditFlags_NoOptions);
    ImGui::ColorEdit4("ObjectColor##2", (float*)&app->entities[selectedEntity].color[0], miscFlags);
}
void LightHierarchyGUI(App* app)
{
    if (ImGui::CollapsingHeader("Lights Hierarchy", ImGuiTreeNodeFlags_None))
    {
        const i64 lightsCount = (i64)app->lights.size();
        for (int i = 0; i < lightsCount; i++)
        {
            // Disable the default "open on single-click behavior" + set Selected flag according to our selection.
            ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
            const bool isSelected = selectedLight == i;
            if (isSelected)
                nodeFlags |= ImGuiTreeNodeFlags_Selected;
            nodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen; // ImGuiTreeNodeFlags_Bullet
            ImGui::TreeNodeEx((void*)(intptr_t)i, nodeFlags, "%s %s %d", LightTypeNames[(i32)app->lights[i].type], glm::to_string(app->lights[i].color).c_str(), i);
            if (ImGui::IsItemClicked())
                selectedLight = i;
        }
    }
}
void LightTransformGUI(App* app) {
    ImGui::Text("Light Transform: %s %s", LightTypeNames[(i32)app->lights[selectedLight].type], glm::to_string(app->lights[selectedLight].color).c_str());
    ImGui::InputFloat3("Position##Light", &app->lights[selectedLight].position[0]);
    ImGui::InputFloat3("Direction", &app->lights[selectedLight].direction[0]);
    
    constexpr bool alphaPreview = true;
    constexpr bool alphaHalfPreview = false;
    constexpr bool optionsMenu = true;
    // ImGui::Checkbox("With Alpha Preview", &alphaPreview);
    // ImGui::Checkbox("With Half Alpha Preview", &alphaHalfPreview);
    // ImGui::Checkbox("With Options Menu", &optionsMenu);
    constexpr ImGuiColorEditFlags miscFlags = (alphaHalfPreview ? ImGuiColorEditFlags_AlphaPreviewHalf : (alphaPreview ? ImGuiColorEditFlags_AlphaPreview : 0)) | (optionsMenu ? 0 : ImGuiColorEditFlags_NoOptions);
    ImGui::ColorEdit4("LightColor##1", (float*)&app->lights[selectedLight].color[0], miscFlags);
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
    LightTransformGUI(app);
    u32& itemCurrentIdx = app->entities[selectedEntity].programIndex;                    // Here our selection data is an index.
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
    ImGui::Separator();
    EntityHierarchyGUI(app);
    LightHierarchyGUI(app);
    ImGui::End();
}

void Update(App* app)
{
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
    
    app->projectionMat = glm::perspective(glm::radians(app->camera.zoom), (float)app->displaySize.x / (float)app->displaySize.y, 0.1f, 100.0f);
    
    CheckShadersHotReload(app);

    PushTransformDataToShader(app);
    PushLightDataToShader(app);
}

void Render(App* app)
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
                const Entity& entity = app->entities[e];
                
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
                    glUniform1i(app->defaultShaderProgram_uTexture, 0); // stackoverflow.com/questions/23687102/gluniform1f-vs-gluniform1i-confusion

                    glDrawElements(GL_TRIANGLES, static_cast<u32>(subMesh.indices.size()), GL_UNSIGNED_INT, (void*)static_cast<u64>(subMesh.indexOffset));
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

void CreateEntity(App* app, const glm::mat4& worldMatrix, const u32 modelIndex, const char* name, const glm::vec4& color, const u32 programIdx)
{
    Entity entity = {};
    entity.name = name;
    entity.worldMatrix = worldMatrix;
    entity.modelIndex = modelIndex;
    entity.color = color;
    entity.programIndex = programIdx;
    
    app->entities.emplace_back(entity);
}

void CreateLight(App* app, const glm::vec3& position, const LightType type, const glm::vec3& dir, const glm::vec4& color)
{
    Light light = {};
    light.type = type;
    light.position = position;
    light.direction = dir;
    light.color = color;

    app->lights.emplace_back(light);
}

void PushTransformDataToShader(App* app)
{
    Buffer& uniformBuffer = app->uniformBuffer;
    BufferManagement::MapBuffer(uniformBuffer, GL_READ_WRITE);

    std::vector<Entity>& entities = app->entities;
    const u64 entityCount = app->entities.size();
    for (u32 i = 0; i < entityCount; ++i)
    {
        Entity& entity = entities[i];
        BufferManagement::AlignHead(uniformBuffer, sizeof(vec4));
        entity.localParamsOffset = uniformBuffer.head;

        PUSH_VEC4(uniformBuffer, entity.color);
        PUSH_MAT4(uniformBuffer, entity.worldMatrix);
        entity.worldViewProjectionMat = app->projectionMat * app->camera.GetViewMatrix() * entity.worldMatrix;
        PUSH_MAT4(uniformBuffer, entity.worldViewProjectionMat);

        entity.localParamsSize = uniformBuffer.head - entity.localParamsOffset;
    }
    
    BufferManagement::UnmapBuffer(uniformBuffer);
}
void PushLightDataToShader(App* app)
{
    Buffer& uniformBuffer = app->uniformBuffer;

    app->globalParamsOffset = uniformBuffer.head;

    const u32 lightsCount = (u32)app->lights.size();
    PUSH_VEC3(uniformBuffer, app->camera.position);
    PUSH_U_INT(uniformBuffer, lightsCount);
    for (u32 i = 0; i < lightsCount; ++i)
    {
        BufferManagement::AlignHead(uniformBuffer, sizeof(vec4));

        Light& light = app->lights[i];
        PUSH_U_INT(uniformBuffer, (u32)light.type);
        PUSH_VEC3(uniformBuffer, light.color);
        PUSH_VEC3(uniformBuffer, light.direction);
        PUSH_VEC3(uniformBuffer, light.position);
    }

    app->globalParamsSize = uniformBuffer.head - app->globalParamsOffset;
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
                glVertexAttribPointer(index, nComp, GL_FLOAT, GL_FALSE, stride, (void*)(u64)offset);
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
