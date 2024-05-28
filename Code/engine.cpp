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

    // Default Texture loading
    app->defaultTextureIdx = TextureSupport::LoadTexture2D(app, "color_white.png");

    // Create uniform buffer
    app->uniformBuffer = CREATE_CONSTANT_BUFFER(BufferManagement::maxUniformBufferSize, nullptr);

    // Create frame buffer and a color texture attachment
    app->frameBufferObject = FrameBufferManagement::CreateFrameBuffer();
    app->gColorTextureIdx = TextureSupport::CreateEmptyColorTexture8Bit(app, "FBO Color", app->displaySizeCurrent.x, app->displaySizeCurrent.y);
    app->gPositionTextureIdx = TextureSupport::CreateEmptyColorTexture16BitF(app, "FBO Position", app->displaySizeCurrent.x, app->displaySizeCurrent.y);
    app->gNormalTextureIdx = TextureSupport::CreateEmptyColorTexture16BitF(app, "FBO Normal", app->displaySizeCurrent.x, app->displaySizeCurrent.y);
    app->gFinalResultTextureIdx = TextureSupport::CreateEmptyColorTexture8Bit(app, "FBO Final Result", app->displaySizeCurrent.x, app->displaySizeCurrent.y);
    app->gDepthTextureIdx = TextureSupport::CreateEmptyDepthTexture(app, "FBO Depth", app->displaySizeCurrent.x, app->displaySizeCurrent.y);

    FrameBufferManagement::BindFrameBuffer(app->frameBufferObject);
    FrameBufferManagement::SetColorAttachment(app->frameBufferObject, app->textures[app->gColorTextureIdx].handle, RT_LOCATION_COLOR);
    FrameBufferManagement::SetColorAttachment(app->frameBufferObject, app->textures[app->gPositionTextureIdx].handle, RT_LOCATION_POSITION);
    FrameBufferManagement::SetColorAttachment(app->frameBufferObject, app->textures[app->gNormalTextureIdx].handle, RT_LOCATION_NORMAL);
    FrameBufferManagement::SetColorAttachment(app->frameBufferObject, app->textures[app->gFinalResultTextureIdx].handle, RT_LOCATION_FINAL_RESULT);
    FrameBufferManagement::SetDepthAttachment(app->frameBufferObject, app->textures[app->gDepthTextureIdx].handle);
    FrameBufferManagement::CheckStatus();
    const std::vector<u32> attachments = { RT_LOCATION_COLOR, RT_LOCATION_POSITION, RT_LOCATION_NORMAL, RT_LOCATION_FINAL_RESULT};
    FrameBufferManagement::SetDrawBuffersTextures(attachments);
    FrameBufferManagement::UnBindFrameBuffer(app->frameBufferObject);
    
    // Camera & View init
    app->projectionMat = glm::perspective(glm::radians(app->camera.zoom), (float)app->displaySizeCurrent.x / (float)app->displaySizeCurrent.y, 0.1f, 100.0f);
    
    // Programs (and retrieve uniform indices)
    const u32 litTexturedProgramIdx = ShaderSupport::LoadProgram(app, "Shaders\\shader_lit_textured.vert", "Shaders\\shader_lit_textured.frag", "LIT_TEXTURED");
    const u32 litBaseProgramIdx = ShaderSupport::LoadProgram(app, "Shaders\\shader_lit_base.vert", "Shaders\\shader_lit_base.frag", "LIT_BASE");

    const u32 unlitBaseProgramIdx = ShaderSupport::LoadProgram(app, "Shaders\\shader_unlit_base.vert", "Shaders\\shader_unlit_base.frag", "UNLIT_BASE");
    const u32 unlitTexturedProgramIdx = ShaderSupport::LoadProgram(app, "Shaders\\shader_unlit_textured.vert", "Shaders\\shader_unlit_textured.frag", "UNLIT_TEXTURED");

    app->deferredGeometryProgramIdx = ShaderSupport::LoadProgram(app, "Shaders\\shader_deferred_geometry_pass.vert", "Shaders\\shader_deferred_geometry_pass.frag", "DEFERRED_GEOMETRY_PASS");
    app->deferredShadingProgramIdx = ShaderSupport::LoadProgram(app, "Shaders\\shader_deferred_shading_pass.vert", "Shaders\\shader_deferred_shading_pass.frag", "DEFERRED_SHADING_PASS");

    app->screenDisplayProgramIdx = ShaderSupport::LoadProgram(app, "Shaders\\shader_unlit_screen.vert", "Shaders\\shader_unlit_screen.frag", "UNLIT_SCREEN");
    
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
    
    // Load models
    const u32 patrickModelIdx = AssimpSupport::LoadModel(app, "Patrick\\Patrick.obj");
    app->quadModel = CreateSampleMesh(app);
    const u32 cubeModelIdx = AssimpSupport::LoadModel(app, "Primitives\\Cube.obj");
    const u32 sphereModelIdx = AssimpSupport::LoadModel(app, "Primitives\\Sphere.obj");

    const u32 arrowsModelIdx = AssimpSupport::LoadModel(app, "Primitives\\Arrows.obj");
    
    const u32 sponzaModelIdx = AssimpSupport::LoadModel(app, "Sponza\\sponza.obj");
    
    Attenuation attenuation = {0.1f, 0.2f, 0.2f};

    CreateLight(app, LightType::DIRECTIONAL, attenuation, glm::vec3(0.0f, 2.0f, 0.0f), glm::vec3(75.0f, 0.0f, 160.0f), glm::vec3(1.0f)
        , cubeModelIdx, unlitBaseProgramIdx, glm::vec4(0.318f), "Directional Light");

    CreateLight(app, LightType::POINT, attenuation, glm::vec3(0.0f, 5.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.5f)
        , sphereModelIdx, unlitBaseProgramIdx, glm::vec4(0.955f, 1.0f, 0.5f, 1.0f), "Point Light 1");

    CreateLight(app, LightType::POINT, attenuation, glm::vec3(-10.0f, 5.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.5f)
        , sphereModelIdx, unlitBaseProgramIdx, glm::vec4(1.0f, 0.5f, 0.5f, 1.0f), "Point Light 2");

    CreateLight(app, LightType::POINT, attenuation, glm::vec3(10.0f, 5.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.5f)
        , sphereModelIdx, unlitBaseProgramIdx, glm::vec4(0.5f, 1.0f, 0.5f, 1.0f), "Point Light 3");
    // Create entities
    // CreateEntity(app, glm::vec3(-2.0f, 10.0f, -15.0f), glm::vec3(-90.0f, 0.0f, 0.0f),glm::vec3(3.0f)
    //     ,sampleMeshModelIdx, unlitTexturedProgramIdx, glm::vec4(1.0f), "SampleModel");
    
    CreateEntity(app, glm::vec3(0.0f, 4.0f, 0.0f), glm::vec3(0.0f),glm::vec4(0.025f)
        ,sponzaModelIdx, litTexturedProgramIdx, glm::vec4(1.0f), "Sponza");
    
    CreateEntity(app, glm::vec3(0.0f, 4.0f, 0.0f), glm::vec3(0.0f),glm::vec3(1.0f)
        ,cubeModelIdx, litTexturedProgramIdx, glm::vec4(1.0f), "Cube");
    
    CreateEntity(app, glm::vec3(0.0f, 5.0f, -4.0f), glm::vec3(0.0f),glm::vec3(0.3f)
        ,patrickModelIdx, litTexturedProgramIdx, glm::vec4(0.788f, 0.522f, 0.02f, 1.0f), "PatrickModel");

    CreateEntity(app, glm::vec3(0.0f, 5.0f, 3.0f), glm::vec3(0.0f, 180.0f, 0.0f),glm::vec3(0.3f)
        ,patrickModelIdx, litTexturedProgramIdx, glm::vec4(0.788f, 0.522f, 0.02f, 1.0f), "PatrickModel");
    
    //  CreateEntity(app, glm::vec3(-6.0f, 4.0f, 0.0f), glm::vec3(0.0f),glm::vec3(1.0f)
    //      ,patrickModelIdx, litBaseProgramIdx, glm::vec4(0.788f, 0.522f, 0.02f, 1.0f), "PatrickModel");
    //  CreateEntity(app, glm::vec3(0.0f, 4.0f, 0.0f), glm::vec3(0.0f),glm::vec3(1.0f)
    //      ,patrickModelIdx, unlitTexturedProgramIdx, glm::vec4(0.788f, 0.522f, 0.02f, 1.0f), "PatrickModel");
    //  CreateEntity(app, glm::vec3(6.0f, 4.0f, 0.0f), glm::vec3(0.0f),glm::vec3(1.0f)
    //     ,patrickModelIdx, litTexturedProgramIdx, glm::vec4(0.788f, 0.522f, 0.02f, 1.0f), "PatrickModel");


    // Set camera intial pos
    app->camera.position = glm::vec3(28.0f, 8.453f, 0.052f);
    app->camera.angles = glm::vec3(-183.0f, -8.1, 0.0f);

    OnScreenResize(app);
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

    if (renderingModeSelection == RenderingMode::DEFERRED)
    {
        // G Buffer Mode
        int gBufferModeSelection = static_cast<int>(app->gBufferMode);
        ImGui::Combo("G-Buffer Mode", &gBufferModeSelection, GBufferModeStr, IM_ARRAYSIZE(GBufferModeStr));
        app->gBufferMode = static_cast<GBufferMode>(gBufferModeSelection);
    }
    
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

        // Light input
        if (std::shared_ptr<Light> light = std::dynamic_pointer_cast<Light>(app->entities[selectedEntity]))
        {
            ImGui::SliderFloat("Attenuation constant", &light->attenuation.constant, 0.0f, 10.0f);
            ImGui::SliderFloat("Attenuation linear", &light->attenuation.linear, 0.0f, 10.0f);
            ImGui::SliderFloat("Attenuation quadratic", &light->attenuation.quadratic, 0.0f, 10.0f);
        }
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
static void PushStyleCompact()
{
    ImGuiStyle& style = ImGui::GetStyle();
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(style.FramePadding.x, (float)(int)(style.FramePadding.y * 0.60f)));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(style.ItemSpacing.x, (float)(int)(style.ItemSpacing.y * 0.60f)));
}

static void PopStyleCompact()
{
    ImGui::PopStyleVar(2);
}

void ResourcesGUI(const App* app)
{
    ImGui::Begin("Resources");
    if (ImGui::CollapsingHeader("Frame Buffer", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Display Pos: ");
        ImGui::SameLine();
        ImGui::Text("%d, %d", app->displayPos.x, app->displayPos.y);
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Previous Display Size: ");
        ImGui::SameLine();
        ImGui::Text("%d, %d", app->displaySizePrevious.x, app->displaySizePrevious.y);
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Current Display Size: ");
        ImGui::SameLine();
        ImGui::Text("%d, %d", app->displaySizeCurrent.x, app->displaySizeCurrent.y);
    }
    if (ImGui::CollapsingHeader("Models", ImGuiTreeNodeFlags_DefaultOpen))
    {
        PushStyleCompact();
        static ImGuiTableFlags flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;
        if (ImGui::BeginTable("Models table", 4, flags))
        {
            ImGui::TableSetupColumn("Idx vector", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Mesh Idx", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Material Idxs", ImGuiTableColumnFlags_WidthFixed);

            ImGui::TableHeadersRow();
            for (int row = 0; row < app->models.size(); row++)
            {
                const Model& model = app->models.at(row);
                ImGui::TableNextRow();
                
                ImGui::TableNextColumn(); ImGui::Text((const char*)std::to_string(row).c_str());
                ImGui::TableNextColumn(); ImGui::Text((const char*)model.name.c_str());
                ImGui::TableNextColumn(); ImGui::Text((const char*)std::to_string(model.meshIdx).c_str());

                std::string materialIdxsStr = "";
                for(i32 i = 0; i < model.materialIdx.size(); ++i)
                {
                    materialIdxsStr += " ";
                    materialIdxsStr += std::to_string(model.materialIdx[i]).c_str();
                    materialIdxsStr += ",";
                }
                
                ImGui::TableNextColumn(); ImGui::Text((const char*)materialIdxsStr.c_str());
            }
            ImGui::EndTable();
            PopStyleCompact();
        }
    }
    if (ImGui::CollapsingHeader("Meshes", ImGuiTreeNodeFlags_DefaultOpen))
    {
        PushStyleCompact();
        static ImGuiTableFlags flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;
        if (ImGui::BeginTable("Meshes table", 5, flags))
        {
            ImGui::TableSetupColumn("Idx vector", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("SubMeshes", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Vertex buffer", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Index buffer", ImGuiTableColumnFlags_WidthStretch);

            ImGui::TableHeadersRow();
            for (int row = 0; row < app->meshes.size(); row++)
            {
                const Mesh& mesh = app->meshes.at(row);
                ImGui::TableNextRow();
                
                ImGui::TableNextColumn(); ImGui::Text((const char*)std::to_string(row).c_str());
                ImGui::TableNextColumn(); ImGui::Text((const char*)mesh.name.c_str());
                ImGui::TableNextColumn();
                if (ImGui::BeginTable("SubMeshes table", 5, flags))
                {
                    ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
                    ImGui::TableSetupColumn("Vertices count", ImGuiTableColumnFlags_WidthStretch);
                    ImGui::TableSetupColumn("Indices count", ImGuiTableColumnFlags_WidthStretch);
                    ImGui::TableSetupColumn("Vertex offset", ImGuiTableColumnFlags_WidthStretch);
                    ImGui::TableSetupColumn("Index offset", ImGuiTableColumnFlags_WidthStretch);

                    ImGui::TableHeadersRow();
                    for (int rowSubMesh = 0; rowSubMesh < mesh.subMeshes.size(); rowSubMesh++)
                    {
                        const SubMesh& subMesh = mesh.subMeshes[rowSubMesh];
                        ImGui::TableNextColumn(); ImGui::Text((const char*)subMesh.name.c_str());
                        ImGui::TableNextColumn(); ImGui::Text((const char*)std::to_string(subMesh.vertices.size()).c_str());
                        ImGui::TableNextColumn(); ImGui::Text((const char*)std::to_string(subMesh.indices.size()).c_str());
                        ImGui::TableNextColumn(); ImGui::Text((const char*)std::to_string(subMesh.vertexOffset).c_str());
                        ImGui::TableNextColumn(); ImGui::Text((const char*)std::to_string(subMesh.indexOffset).c_str());
                    }
                    ImGui::EndTable();
                }
                ImGui::TableNextColumn();
                if (ImGui::BeginTable("VBO table", 3, flags))
                {
                    ImGui::TableSetupColumn("Handle", ImGuiTableColumnFlags_WidthStretch);
                    ImGui::TableSetupColumn("Head", ImGuiTableColumnFlags_WidthStretch);
                    ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthStretch);
                    ImGui::TableHeadersRow();

                    ImGui::TableNextColumn(); ImGui::Text((const char*)std::to_string(mesh.vertexBuffer.handle).c_str());
                    ImGui::TableNextColumn(); ImGui::Text((const char*)std::to_string(mesh.vertexBuffer.head).c_str());
                    ImGui::TableNextColumn(); ImGui::Text((const char*)std::to_string(mesh.vertexBuffer.size).c_str());
                    ImGui::EndTable();
                }
                
                ImGui::TableNextColumn();
                if (ImGui::BeginTable("EBO table", 3, flags))
                {
                    ImGui::TableSetupColumn("Handle", ImGuiTableColumnFlags_WidthStretch);
                    ImGui::TableSetupColumn("Head", ImGuiTableColumnFlags_WidthStretch);
                    ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthStretch);
                    ImGui::TableHeadersRow();

                    ImGui::TableNextColumn(); ImGui::Text((const char*)std::to_string(mesh.indexBuffer.handle).c_str());
                    ImGui::TableNextColumn(); ImGui::Text((const char*)std::to_string(mesh.indexBuffer.head).c_str());
                    ImGui::TableNextColumn(); ImGui::Text((const char*)std::to_string(mesh.indexBuffer.size).c_str());
                    ImGui::EndTable();
                }
                
            }
            ImGui::EndTable();
            PopStyleCompact();
        }
    }
    if (ImGui::CollapsingHeader("Textures", ImGuiTreeNodeFlags_DefaultOpen))
    {
        constexpr i32 minTexVisSize = 50;
        constexpr i32 maxTexVisSize = 200;
        static i32 textVisSize = 50;
        ImGui::SliderInt("Texture visualization size", &textVisSize, minTexVisSize, maxTexVisSize);
        PushStyleCompact();
        static ImGuiTableFlags flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;
        if (ImGui::BeginTable("Textures table", 5, flags))
        {
            ImGui::TableSetupColumn("Idx vector", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Handle", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Path", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Image", ImGuiTableColumnFlags_WidthStretch);

            ImGui::TableHeadersRow();
            for (int row = 0; row < app->textures.size(); row++)
            {
                const Texture& tex = app->textures.at(row);
                ImGui::TableNextRow();
                ImGui::TableNextColumn(); ImGui::Text((const char*)std::to_string(row).c_str());
                ImGui::TableNextColumn(); ImGui::Text((const char*)std::to_string(tex.handle).c_str());
                ImGui::TableNextColumn(); ImGui::Text((const char*)TextureTypeStr[(int)tex.type]);
                ImGui::TableNextColumn(); ImGui::Text((const char*)tex.path.c_str());
                ImGui::TableNextColumn();
                constexpr ImVec2 uvMin = ImVec2(0.0f, 1.0f);                 // Top-left
                constexpr ImVec2 uvMax = ImVec2(1.0f, 0.0f);                 // Lower-right
                constexpr ImVec4 tintCol = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // No tint
                ImVec4 borderCol = ImGui::GetStyleColorVec4(ImGuiCol_Border);
                ImGui::Image((void*)tex.handle, ImVec2(textVisSize, textVisSize), uvMin, uvMax, tintCol, borderCol);
                ImGui::SameLine();
                std::string info = TextureSupport::GetInfoString(tex);
                ImGui::Text(info.c_str());
            }
            ImGui::EndTable();
            PopStyleCompact();
        }
    }
    if (ImGui::CollapsingHeader("Materials", ImGuiTreeNodeFlags_DefaultOpen))
    {
        PushStyleCompact();
        static ImGuiTableFlags flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;
        if (ImGui::BeginTable("Materials table", 10, flags))
        {
            ImGui::TableSetupColumn("Idx vector", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Albedo", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Emissive", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Smoothness", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("AlbedoTextureIdx", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("EmissiveTextureIdx", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("SpecularTextureIdx", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("NormalsTextureIdx", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("BumpTextureIdx", ImGuiTableColumnFlags_WidthFixed);

            ImGui::TableHeadersRow();
            for (int row = 0; row < app->materials.size(); row++)
            {
                const Material& mat = app->materials.at(row);
                ImGui::TableNextRow();
                
                ImGui::TableNextColumn(); ImGui::Text((const char*)std::to_string(row).c_str());
                ImGui::TableNextColumn(); ImGui::Text((const char*)mat.name.c_str());
                ImGui::TableNextColumn(); ImGui::ColorButton("Albedo", *(ImVec4*) glm::value_ptr(mat.albedo), ImGuiColorEditFlags_NoPicker);
                ImGui::TableNextColumn(); ImGui::ColorButton("Emissive", *(ImVec4*) glm::value_ptr(mat.emissive), ImGuiColorEditFlags_NoPicker);
                ImGui::TableNextColumn(); ImGui::Text((const char*)std::to_string(mat.smoothness).c_str());
                ImGui::TableNextColumn(); ImGui::Text((const char*)std::to_string(mat.albedoTextureIdx).c_str());
                ImGui::TableNextColumn(); ImGui::Text((const char*)std::to_string(mat.emissiveTextureIdx).c_str());
                ImGui::TableNextColumn(); ImGui::Text((const char*)std::to_string(mat.specularTextureIdx).c_str());
                ImGui::TableNextColumn(); ImGui::Text((const char*)std::to_string(mat.normalsTextureIdx).c_str());
                ImGui::TableNextColumn(); ImGui::Text((const char*)std::to_string(mat.bumpTextureIdx).c_str());
            }
            ImGui::EndTable();
            PopStyleCompact();
        }
    }
    if (ImGui::CollapsingHeader("Programs", ImGuiTreeNodeFlags_DefaultOpen))
    {
    }
    ImGui::End();
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
    app->projectionMat = glm::perspective(glm::radians(app->camera.zoom), (float)app->displaySizeCurrent.x / (float)app->displaySizeCurrent.y, 0.1f, 100.0f);

    // Programs hot reload
    CheckShadersHotReload(app);

    // Uniform buffers push
    Buffer& uniformBuffer = app->uniformBuffer;
    BufferManagement::MapBuffer(uniformBuffer, GL_READ_WRITE);
    PushTransformUBO(app);
    PushLightDataUBO(app);
    PushMaterialDataUBO(app);
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
        ForwardRenderLightBoxes(app);
        break;
    }
    ResourcesGUI(app);
}
  
void ForwardRender(App* app)
{
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 1, -1, "Engine Render");

    // Render on this framebuffer render targets
    FrameBufferManagement::BindFrameBuffer(app->frameBufferObject);

    // Select on which render targets to draw
    const std::vector<u32> attachments = { 0 };
    FrameBufferManagement::SetDrawBuffersTextures(attachments);
    
    glEnable(GL_DEPTH_TEST);
    
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glViewport(0, 0, app->displaySizeCurrent.x, app->displaySizeCurrent.y);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
            // - clear the framebuffer
            // - set the viewport
            // - set the blending state
            // - bind the texture into unit 0
            // - bind the program 
            //   (...and make its texture sample from unit 0)
            // - bind the vao
            // - glDrawElements() !!!

    BufferManagement::BindBufferRange(app->uniformBuffer, STD_140_BINDING_POINT::BP_GLOBAL_PARAMS, app->globalParamsSize, app->globalParamsOffset);

    const u32 entityCount = static_cast<u32>(app->entities.size());
    for (u32 e = 0; e < entityCount; ++e)
    {
        const Entity& entity = *app->entities[e];
                
        const Program& program = app->programs[entity.programIndex];
        app->defaultShaderProgram_uTexture = glGetUniformLocation(program.handle, "uTexture");
        glUseProgram(program.handle);
        BufferManagement::BindBufferRange(app->uniformBuffer, STD_140_BINDING_POINT::BP_LOCAL_PARAMS, entity.localParamsSize, entity.localParamsOffset);

        Model& model = app->models[entity.modelIndex];
        Mesh& mesh = app->meshes[model.meshIdx];
                
        glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 1, -1, model.name.c_str());

        const u32 subMeshCount = static_cast<u32>(mesh.subMeshes.size());
        for (u32 i = 0; i < subMeshCount; i++)
        {
            const u32 subMeshMaterialIdx = model.materialIdx[i];
            const Material subMeshMaterial = app->materials[subMeshMaterialIdx];
            BufferManagement::BindBufferRange(app->uniformBuffer, STD_140_BINDING_POINT::BP_MATERIAL_PARAMS, subMeshMaterial.paramsSize, subMeshMaterial.paramsOffset);

            const std::vector<u32> texturesUniformLocations = { MAT_T_DIFFUSE, MAT_T_BUMP, MAT_T_SPECULAR };
            const std::vector<u32> texturesUniformHandles = { app->textures[subMeshMaterial.albedoTextureIdx].handle, app->textures[subMeshMaterial.bumpTextureIdx].handle,
                app->textures[subMeshMaterial.specularTextureIdx].handle};
            
            mesh.DrawSubMesh(i, texturesUniformHandles, texturesUniformLocations, program, false);
        }
        glPopDebugGroup();
    }
    glPopDebugGroup();
    
    FrameBufferManagement::UnBindFrameBuffer(app->frameBufferObject);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(0, 0, app->displaySizeCurrent.x, app->displaySizeCurrent.y);
    glDisable(GL_DEPTH_TEST);
    
    const Program& program = app->programs[app->screenDisplayProgramIdx];
    app->defaultShaderProgram_uTexture = glGetUniformLocation(program.handle, "uTexture");
    glUseProgram(program.handle);
    Model& model = app->models[app->quadModel];
    Mesh& mesh = app->meshes[model.meshIdx];
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 1, -1, model.name.c_str());
    const u32 subMeshCount = static_cast<u32>(mesh.subMeshes.size());
    for (u32 i = 0; i < subMeshCount; i++)
    {
        const u32 subMeshMaterialIdx = model.materialIdx[i];
        const Material subMeshMaterial = app->materials[subMeshMaterialIdx];
        BufferManagement::BindBufferRange(app->uniformBuffer, STD_140_BINDING_POINT::BP_MATERIAL_PARAMS, subMeshMaterial.paramsSize, subMeshMaterial.paramsOffset);
        mesh.DrawSubMesh(i, app->textures[subMeshMaterial.albedoTextureIdx], app->defaultShaderProgram_uTexture, program, false);
    }
    glPopDebugGroup();
}

void ForwardRenderLightBoxes(App* app)
{
    if (app->gBufferMode !=  GBufferMode::FINAL)
        return;
    
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 1, -1, "Engine Forward Render Light Boxes");

    // Render on this framebuffer render targets
    FrameBufferManagement::BindFrameBuffer(app->frameBufferObject);

    // Select on which render targets to draw
    const std::vector<u32> attachments = { RT_LOCATION_FINAL_RESULT };
    FrameBufferManagement::SetDrawBuffersTextures(attachments);

    glEnable(GL_DEPTH_TEST);

    //glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glViewport(0, 0, app->displaySizeCurrent.x, app->displaySizeCurrent.y);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // - clear the framebuffer
    // - set the viewport
    // - set the blending state
    // - bind the texture into unit 0
    // - bind the program 
    //   (...and make its texture sample from unit 0)
    // - bind the vao
    // - glDrawElements() !!!

    BufferManagement::BindBufferRange(app->uniformBuffer, STD_140_BINDING_POINT::BP_GLOBAL_PARAMS, app->globalParamsSize, app->globalParamsOffset);

    const u32 entityCount = static_cast<u32>(app->entities.size());
    for (u32 e = 0; e < entityCount; ++e)
    {
        const Entity& entity = *app->entities[e];
        // Skip lights' geometry in deferred rendering, draw them only in forward
        if (std::shared_ptr<Light> light = std::dynamic_pointer_cast<Light>(app->entities[e]))
        {
            const Program& program = app->programs[entity.programIndex];
            app->defaultShaderProgram_uTexture = glGetUniformLocation(program.handle, "uTexture");
            glUseProgram(program.handle);
            BufferManagement::BindBufferRange(app->uniformBuffer, STD_140_BINDING_POINT::BP_LOCAL_PARAMS, entity.localParamsSize, entity.localParamsOffset);

            Model& model = app->models[entity.modelIndex];
            Mesh& mesh = app->meshes[model.meshIdx];

            glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 1, -1, model.name.c_str());

            const u32 subMeshCount = static_cast<u32>(mesh.subMeshes.size());
            for (u32 i = 0; i < subMeshCount; i++)
            {
                const u32 subMeshMaterialIdx = model.materialIdx[i];
                const Material subMeshMaterial = app->materials[subMeshMaterialIdx];
                BufferManagement::BindBufferRange(app->uniformBuffer, STD_140_BINDING_POINT::BP_MATERIAL_PARAMS, subMeshMaterial.paramsSize, subMeshMaterial.paramsOffset);
                mesh.DrawSubMesh(i, app->textures[app->gFinalResultTextureIdx], app->defaultShaderProgram_uTexture, program, false);
            }
            glPopDebugGroup();
        }
    }

    FrameBufferManagement::UnBindFrameBuffer(app->frameBufferObject);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(0, 0, app->displaySizeCurrent.x, app->displaySizeCurrent.y);
    glDisable(GL_DEPTH_TEST);

    const Program& program = app->programs[app->screenDisplayProgramIdx];
    app->defaultShaderProgram_uTexture = glGetUniformLocation(program.handle, "uTexture");
    glUseProgram(program.handle);
    Model& model = app->models[app->quadModel];
    Mesh& mesh = app->meshes[model.meshIdx];
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 1, -1, model.name.c_str());
    const u32 subMeshCount = static_cast<u32>(mesh.subMeshes.size());
    for (u32 i = 0; i < subMeshCount; i++)
    {
        const u32 subMeshMaterialIdx = model.materialIdx[i];
        const Material subMeshMaterial = app->materials[subMeshMaterialIdx];
        BufferManagement::BindBufferRange(app->uniformBuffer, STD_140_BINDING_POINT::BP_MATERIAL_PARAMS, subMeshMaterial.paramsSize, subMeshMaterial.paramsOffset);
        mesh.DrawSubMesh(i, app->textures[app->gFinalResultTextureIdx], app->defaultShaderProgram_uTexture, program, false);
    }
    glPopDebugGroup();

    glPopDebugGroup();
}

void DeferredRender(App* app) {
    DeferredRenderGeometryPass(app);
    DeferredRenderShadingPass(app);
    DeferredRenderDisplayPass(app);
}

void DeferredRenderGeometryPass(App* app)
{
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 1, -1, "Engine Deferred Render Geometry");

    // Render on this framebuffer render targets
    FrameBufferManagement::BindFrameBuffer(app->frameBufferObject);

    // Select on which render targets to draw
    const std::vector<u32> attachments = { RT_LOCATION_COLOR, RT_LOCATION_POSITION, RT_LOCATION_NORMAL};
    FrameBufferManagement::SetDrawBuffersTextures(attachments);

    glEnable(GL_DEPTH_TEST);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glViewport(0, 0, app->displaySizeCurrent.x, app->displaySizeCurrent.y);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // - clear the framebuffer
    // - set the viewport
    // - set the blending state
    // - bind the texture into unit 0
    // - bind the program 
    //   (...and make its texture sample from unit 0)
    // - bind the vao
    // - glDrawElements() !!!

    BufferManagement::BindBufferRange(app->uniformBuffer, STD_140_BINDING_POINT::BP_GLOBAL_PARAMS, app->globalParamsSize, app->globalParamsOffset);

    // Bind the deferred program
    const Program& program = app->programs[app->deferredGeometryProgramIdx];
    app->defaultShaderProgram_uTexture = glGetUniformLocation(program.handle, "uTexture");
    glUseProgram(program.handle);

    const u32 entityCount = static_cast<u32>(app->entities.size());
    for (u32 e = 0; e < entityCount; ++e)
    {
        const Entity& entity = *app->entities[e];

        // Skip lights' geometry in deferred rendering, draw them only in forward
        if (std::shared_ptr<Light> light = std::dynamic_pointer_cast<Light>(app->entities[e]))
        {
            continue;
        }        
        BufferManagement::BindBufferRange(app->uniformBuffer, STD_140_BINDING_POINT::BP_LOCAL_PARAMS, entity.localParamsSize, entity.localParamsOffset);

        Model& model = app->models[entity.modelIndex];
        Mesh& mesh = app->meshes[model.meshIdx];

        glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 1, -1, model.name.c_str());

        const u32 subMeshCount = static_cast<u32>(mesh.subMeshes.size());
        for (u32 i = 0; i < subMeshCount; i++)
        {
            const u32 subMeshMaterialIdx = model.materialIdx[i];
            const Material subMeshMaterial = app->materials[subMeshMaterialIdx];
            BufferManagement::BindBufferRange(app->uniformBuffer, STD_140_BINDING_POINT::BP_MATERIAL_PARAMS, subMeshMaterial.paramsSize, subMeshMaterial.paramsOffset);
            mesh.DrawSubMesh(i, app->textures[subMeshMaterial.albedoTextureIdx], app->defaultShaderProgram_uTexture, program, false);
        }

        glPopDebugGroup();
    }
    glPopDebugGroup();

    FrameBufferManagement::UnBindFrameBuffer(app->frameBufferObject);
}

void DeferredRenderShadingPass(App* app)
{
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 1, -1, "Engine Deferred Render Shader");
    // Render on this framebuffer render targets
    FrameBufferManagement::BindFrameBuffer(app->frameBufferObject);

    // Select on which render targets to draw
    const std::vector<u32> attachments = { RT_LOCATION_FINAL_RESULT};
    FrameBufferManagement::SetDrawBuffersTextures(attachments);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glViewport(0, 0, app->displaySizeCurrent.x, app->displaySizeCurrent.y);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // - clear the framebuffer
    // - set the viewport
    // - set the blending state
    // - bind the texture into unit 0
    // - bind the program 
    //   (...and make its texture sample from unit 0)
    // - bind the vao
    // - glDrawElements() !!!

    BufferManagement::BindBufferRange(app->uniformBuffer, STD_140_BINDING_POINT::BP_GLOBAL_PARAMS, app->globalParamsSize, app->globalParamsOffset);

    // Bind the deferred program
    const Program& program = app->programs[app->deferredShadingProgramIdx];
    glUseProgram(program.handle);

    const std::vector<u32> texturesUniformLocations = { RT_LOCATION_COLOR, RT_LOCATION_POSITION, RT_LOCATION_NORMAL };
    const std::vector<u32> texturesUniformHandles = { app->textures[app->gColorTextureIdx].handle, app->textures[app->gPositionTextureIdx].handle,
        app->textures[app->gNormalTextureIdx].handle};
    
    Model& model = app->models[app->quadModel];
    Mesh& mesh = app->meshes[model.meshIdx];
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 1, -1, model.name.c_str());
    const u32 subMeshCount = static_cast<u32>(mesh.subMeshes.size());
    for (u32 i = 0; i < subMeshCount; i++)
    {
        const u32 subMeshMaterialIdx = model.materialIdx[i];
        const Material subMeshMaterial = app->materials[subMeshMaterialIdx];
        BufferManagement::BindBufferRange(app->uniformBuffer, STD_140_BINDING_POINT::BP_MATERIAL_PARAMS, subMeshMaterial.paramsSize, subMeshMaterial.paramsOffset);
        mesh.DrawSubMesh(i, texturesUniformHandles, texturesUniformLocations, program, false);
    }
    FrameBufferManagement::UnBindFrameBuffer(app->frameBufferObject);
    glDepthMask(GL_TRUE);
    glPopDebugGroup();
    glPopDebugGroup();
}

void DeferredRenderDisplayPass(App* app)
{
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 1, -1, "Engine Deferred Render Display");

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(0, 0, app->displaySizeCurrent.x, app->displaySizeCurrent.y);
    glDisable(GL_DEPTH_TEST);
    
    // Draw the framebuffer onto a quad that covers the whole screen.
    const Program& screenProgram = app->programs[app->screenDisplayProgramIdx];
    app->defaultShaderProgram_uTexture = glGetUniformLocation(screenProgram.handle, "uTexture");
    glUseProgram(screenProgram.handle);
    Model& model = app->models[app->quadModel];
    Mesh& mesh = app->meshes[model.meshIdx];
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 1, -1, model.name.c_str());
    const u32 subMeshCount = static_cast<u32>(mesh.subMeshes.size());
    for (u32 i = 0; i < subMeshCount; i++)
    {
        // Draw the framebuffer onto a quad that covers the whole screen.
        u32 gBufferModeIdx = 0;
        switch (app->gBufferMode)
        {
        case GBufferMode::COLOR:
            gBufferModeIdx = app->gColorTextureIdx;
            break;
        case GBufferMode::NORMAL:
            gBufferModeIdx = app->gNormalTextureIdx;
            break;
        case GBufferMode::POSITION:
            gBufferModeIdx = app->gPositionTextureIdx;
            break;
        case GBufferMode::DEPTH:
            gBufferModeIdx = app->gDepthTextureIdx;
            break;
        case GBufferMode::FINAL:
            gBufferModeIdx = app->gFinalResultTextureIdx; // Same as app->colorTextureIdx
            break;
        default:
            break;
        }
        const u32 subMeshMaterialIdx = model.materialIdx[i];
        const Material subMeshMaterial = app->materials[subMeshMaterialIdx];
        BufferManagement::BindBufferRange(app->uniformBuffer, STD_140_BINDING_POINT::BP_MATERIAL_PARAMS, subMeshMaterial.paramsSize, subMeshMaterial.paramsOffset);
        mesh.DrawSubMesh(i, app->textures[gBufferModeIdx], app->defaultShaderProgram_uTexture, screenProgram, false);
    }
    glPopDebugGroup();
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

void CreateLight(App* app, LightType lightType, const Attenuation& attenuation, const glm::vec3& position, const glm::vec3& orientation, const glm::vec3& scale, const u32 modelIndex, const u32 programIdx, const glm::vec4& lightColor, const char* name)
{
    std::shared_ptr<Light> light = std::make_shared<Light>();
    light->type = lightType;
    light->eType = EntityType::LIGHT;
    light->attenuation = attenuation;
    light->attenuation.CalculateRadius(lightColor);
    
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

void PushTransformUBO(App* app)
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
void PushLightDataUBO(App* app)
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

        Light& light = *app->lights[i];
        light.attenuation.CalculateRadius(light.color);
        
        PUSH_U_INT(uniformBuffer, (u32)light.type)
        PUSH_VEC3(uniformBuffer, light.color);
        PUSH_VEC3(uniformBuffer, light.orientationEuler);
        PUSH_VEC3(uniformBuffer, light.position);
        PUSH_FLOAT(uniformBuffer, light.attenuation.constant);
        PUSH_FLOAT(uniformBuffer, light.attenuation.linear);
        PUSH_FLOAT(uniformBuffer, light.attenuation.quadratic);
        PUSH_FLOAT(uniformBuffer, light.attenuation.radius);
    }

    // Set buffer block end and set size
    BufferManagement::SetBufferBlockEnd(uniformBuffer, BufferManagement::uniformBlockAlignment, app->globalParamsSize, app->globalParamsOffset);

    if (app->debugUBO)
        std::cout << "Global Params. " << " Offset: " << app->globalParamsOffset << " Size: " << app->globalParamsSize  << "\n";
}
void PushMaterialDataUBO(App* app)
{
    for (u32 i = 0; i < app->materials.size(); ++i)
    {
        Material& material = app->materials[i];
        Buffer& uniformBuffer = app->uniformBuffer;

        // Set buffer block start and set offset
        BufferManagement::SetBufferBlockStart(uniformBuffer, BufferManagement::uniformBlockAlignment, material.paramsOffset);
        PUSH_VEC3(uniformBuffer, material.albedo);
        PUSH_VEC3(uniformBuffer, material.emissive);
        PUSH_FLOAT(uniformBuffer, material.smoothness);
        PUSH_U_INT(uniformBuffer, (material.albedoTextureIdx != 0) ? true : false); 
        PUSH_U_INT(uniformBuffer, (material.emissiveTextureIdx != 0) ? true : false); 
        PUSH_U_INT(uniformBuffer, (material.specularTextureIdx != 0) ? true : false); 
        PUSH_U_INT(uniformBuffer, (material.normalsTextureIdx != 0) ? true : false); 
        PUSH_U_INT(uniformBuffer, (material.bumpTextureIdx != 0) ? true : false); 
        // Set buffer block end and set size
        BufferManagement::SetBufferBlockEnd(uniformBuffer, BufferManagement::uniformBlockAlignment, material.paramsSize, material.paramsOffset);
    }
}

void OnScreenResize(App* app)
{
    std::cout << "Screen resize" << "\n\n";
    //FrameBufferManagement::BindFrameBuffer(app->frameBufferObject);

    // Resizing only for IMGUI shader. Engine application shaders work fine but IMGUI needs resizing.
    for (u32 i = 0; i < app->textures.size(); ++i)
    {
        Texture& tex = app->textures[i];
        if (tex.type != TextureType::NON_FBO)
            TextureSupport::ResizeTexture(app, tex, app->displaySizeCurrent.x, app->displaySizeCurrent.y);
    }
    //FrameBufferManagement::SetColorAttachment(app->frameBufferObject, app->textures[app->colorTextureIdx].handle, 0);
    //// FrameBufferManagement::SetDepthAttachment(app->frameBufferObject, app->textures[app->depthTextureIdx].handle);
    //// FrameBufferManagement::CheckStatus();
    //// const std::vector<u32> attachments = {0};
    //// FrameBufferManagement::SetDrawBuffersTextures(attachments);
    //FrameBufferManagement::UnBindFrameBuffer(app->frameBufferObject);
}

void EditTransform(App* app, const float* cameraView, float* cameraProjection, float* matrix, bool editTransformDecomposition)
{
    if (editTransformDecomposition)
    {
        Entity& entity = *app->entities[selectedEntity];

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
        ImGuizmo::DecomposeMatrixToComponents(matrix, glm::value_ptr(entity.position), glm::value_ptr(entity.orientationEuler), glm::value_ptr(entity.scale));
        ImGui::InputFloat3("Translate", glm::value_ptr(entity.position));
        ImGui::InputFloat3("Rotate", glm::value_ptr(entity.orientationEuler));
        ImGui::InputFloat3("Scale", glm::value_ptr(entity.scale));
        ImGuizmo::RecomposeMatrixFromComponents(glm::value_ptr(entity.position), glm::value_ptr(entity.orientationEuler), glm::value_ptr(entity.scale), matrix);

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
    ImGuizmo::SetRect((float)app->displayPos.x, (float)app->displayPos.y, (float)app->displaySizeCurrent.x, (float)app->displaySizeCurrent.y);
    //ImGuizmo::DrawGrid(cameraView, glm::value_ptr(app->projectionMat), glm::value_ptr(glm::mat4(1.0f)), 100.f);
    ImGuizmo::Manipulate(cameraView, glm::value_ptr(app->projectionMat), app->imGuizmoData.mCurrentGizmoOperation, app->imGuizmoData.mCurrentGizmoMode, glm::value_ptr(app->entities[selectedEntity]->worldMatrix), nullptr, app->imGuizmoData.useSnap ? &app->imGuizmoData.snap[0] : nullptr, app->imGuizmoData.boundSizing ? app->imGuizmoData.bounds : nullptr, app->imGuizmoData.boundSizingSnap ? app->imGuizmoData.boundsSnap : nullptr);

    constexpr bool alphaPreview = true;
    constexpr bool alphaHalfPreview = false;
    constexpr bool optionsMenu = true;
    constexpr ImGuiColorEditFlags miscFlags = (alphaHalfPreview ? ImGuiColorEditFlags_AlphaPreviewHalf : (alphaPreview ? ImGuiColorEditFlags_AlphaPreview : 0)) | (optionsMenu ? 0 : ImGuiColorEditFlags_NoOptions);
    ImGui::ColorEdit4("ObjectColor##2", (float*)&app->entities[selectedEntity]->color[0], miscFlags); 
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
