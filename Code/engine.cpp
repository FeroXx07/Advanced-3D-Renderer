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
    app->whiteTexIdx = TextureSupport::LoadTexture2D(app, "color_white.png");
    app->blackTexIdx = TextureSupport::LoadTexture2D(app, "color_black.png");
    app->normalTexIdx = TextureSupport::LoadTexture2D(app, "color_normal.png");
    app->magentaTexIdx = TextureSupport::LoadTexture2D(app, "color_magenta.png");
    
    // - programs (and retrieve uniform indices)
    //app->texturedGeometryProgramIdx = ShaderSupport::LoadProgram(app, "shaders.glsl", "TEXTURED_GEOMETRY");
    //app->texturedGeometryProgramIdx = ShaderSupport::LoadProgram(app, "shaders.vert", "shaders.frag", "TEXTURED_GEOMETRY");
    app->texturedGeometryProgramIdx = ShaderSupport::LoadProgram(app, "bufferedShader.vert", "bufferedShader.frag", "BUFFERED_SHADER");
    //app->texturedGeometryProgramIdx = ShaderSupport::LoadProgram(app, "shaded.glsl", "SHADED_MODEL");

    // Fill vertex shader layout auto
    Program& program = app->programs[app->texturedGeometryProgramIdx];
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
    
    app->defaultShaderProgram_uTexture = glGetUniformLocation(program.handle, "uTexture");
    app->mode = Mode_TexturedQuad;

    const u32 patrickModelIdx = AssimpSupport::LoadModel(app, "Patrick\\patrick.obj");
    const u32 sampleMeshModelIdx = CreateSampleMesh(app);

    CreateEntity(app, glm::translate(identityMat, glm::vec3(0.0f, -1.0f, 0.0f)), sampleMeshModelIdx, "SampleModel");
    CreateEntity(app, glm::scale(identityMat, glm::vec3(0.3f)), patrickModelIdx, "PatrickModel");
    CreateEntity(app, glm::scale(glm::translate(identityMat, glm::vec3(2.0f, 0.0f, 0.0f)), glm::vec3(0.3f)), patrickModelIdx, "PatrickModel2");
    CreateEntity(app, glm::scale(glm::translate(identityMat, glm::vec3(-2.0f, 0.0f, 0.0f)), glm::vec3(0.3f)), patrickModelIdx, "PatrickModel3");

    PushTransformDataToShader(app);
}

void CameraGUI(App* app) {
    ImGui::Text("Camera");
    ImGui::InputFloat3("Position", &app->camera.position[0]);
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
            const bool isSelected = nodeClicked == i;
            if (isSelected)
                nodeFlags |= ImGuiTreeNodeFlags_Selected;
            nodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen; // ImGuiTreeNodeFlags_Bullet
            ImGui::TreeNodeEx((void*)(intptr_t)i, nodeFlags, "%s %d", app->entities[i].name.c_str(), i);
            if (ImGui::IsItemClicked())
                nodeClicked = i;
        }
    }
}
void EntityTransformGUI(App* app) {
    ImGui::Text("Transform: %s", app->entities[nodeClicked].name.c_str());

    // Decompose model matrix
    glm::vec3 translation, scale, skew;
    glm::vec4 perspective;
    glm::quat orientation;
    glm::decompose(app->entities[nodeClicked].worldMatrix, scale, orientation, translation, skew, perspective);
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
    
        app->entities[nodeClicked].worldMatrix = composedModelMat;
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
    EntityHierarchyGUI(app);
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
            // TODO: Draw your textured quad here!
            // - clear the framebuffer
            // - set the viewport
            // - set the blending state
            // - bind the texture into unit 0
            // - bind the program 
            //   (...and make its texture sample from unit 0)
            // - bind the vao
            // - glDrawElements() !!!

            const Program& program = app->programs[app->texturedGeometryProgramIdx];
            glUseProgram(program.handle);
        
            const u32 entityCount = static_cast<u32>(app->entities.size());
            for (u32 e = 0; e < entityCount; ++e)
            {
                const Entity& entity = app->entities[e];
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
void CreateEntity(App* app, const glm::mat4& worldMatrix, const u32 modelIndex, const char* name)
{
    Entity entity = {};
    entity.name = name;
    entity.worldMatrix = worldMatrix;
    entity.modelIndex = modelIndex;

    app->entities.emplace_back(entity);
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
        
        PUSH_MAT4(uniformBuffer, entity.worldMatrix);
        entity.worldViewProjectionMat = app->projectionMat * app->camera.GetViewMatrix() * entity.worldMatrix;
        PUSH_MAT4(uniformBuffer, entity.worldViewProjectionMat);

        entity.localParamsSize = uniformBuffer.head - entity.localParamsOffset;
    }
    
    BufferManagement::UnmapBuffer(uniformBuffer);
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
