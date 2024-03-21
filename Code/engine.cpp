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
#include <stb_image.h>

#include "assimp_model_loading.h"
#include "program.h"
#include "texture.h"
#include "vertex.h"

void Init(App* app)
{
    app->ctx = RetrieveOpenGLContext();
    
   
    
    // - programs (and retrieve uniform indices)
    //app->texturedGeometryProgramIdx = ShaderSupport::LoadProgram(app, "shaders.glsl", "TEXTURED_GEOMETRY");
    //app->texturedGeometryProgramIdx = ShaderSupport::LoadProgram(app, "shaders.vert", "shaders.frag", "TEXTURED_GEOMETRY");
    //app->texturedGeometryProgramIdx = ShaderSupport::LoadProgram(app, "shaded.vert", "shaded.frag", "SHADED");
    app->texturedGeometryProgramIdx = ShaderSupport::LoadProgram(app, "shaded.glsl", "SHADED_MODEL");

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
    
    app->texturedMeshProgram_uTexture = glGetUniformLocation(program.handle, "uTexture");
    
    // - textures
    app->diceTexIdx = TextureSupport::LoadTexture2D(app, "dice.png");
    app->whiteTexIdx = TextureSupport::LoadTexture2D(app, "color_white.png");
    app->blackTexIdx = TextureSupport::LoadTexture2D(app, "color_black.png");
    app->normalTexIdx = TextureSupport::LoadTexture2D(app, "color_normal.png");
    app->magentaTexIdx = TextureSupport::LoadTexture2D(app, "color_magenta.png");

    app->mode = Mode_TexturedQuad;

    app->model = AssimpSupport::LoadModel(app, "Patrick\\patrick.obj");
}

void Gui(const App* app)
{
    bool showDemoWindow = false;
    ImGui::ShowDemoWindow(&showDemoWindow);
    
    ImGui::Begin("Info");
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
    ImGui::End();
}

void Update(App* app)
{
    // You can handle app->input keyboard/mouse here
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

void Render(App* app)
{
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 1, -1, "Shaded Model");
    
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

            const Model& model = app->models[app->model];
            Mesh& mesh = app->meshes[model.meshIdx];

            const u32 subMeshCount = static_cast<u32>(mesh.subMeshes.size());
            for (u32 i = 0; i < subMeshCount; i++)
            {
                const GLuint vao = VAOSupport::FindVAO(mesh, i, program);
                glBindVertexArray(vao);

                const u32 subMeshMaterialIdx = model.materialIdx[i];
                const Material subMeshMaterial = app->materials[subMeshMaterialIdx];

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, app->textures[subMeshMaterial.albedoTextureIdx].handle);
                glUniform1i(app->texturedMeshProgram_uTexture, 0); // stackoverflow.com/questions/23687102/gluniform1f-vs-gluniform1i-confusion

                SubMesh& subMesh = mesh.subMeshes[i];
                glDrawElements(GL_TRIANGLES, static_cast<u32>(subMesh.indices.size()), GL_UNSIGNED_INT, (void*)static_cast<u64>(subMesh.indexOffset));
            }
        }
        break;
    case Mode_Count:
        break;
    }
    glPopDebugGroup();
}

void VAOSupport::CreateNewVAO(const Mesh& mesh, const SubMesh& subMesh, const Program& program, GLuint& vaoHandle)
{
    std::cout << "Creating new VAO" << "\n" << "\n";
    glGenVertexArrays(1, &vaoHandle);
    glBindVertexArray(vaoHandle);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.vertexBufferHandle);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.indexBufferHandle);

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
