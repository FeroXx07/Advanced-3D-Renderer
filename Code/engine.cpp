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
    
    /* TODO: Initialize your resources here! */
    // - vertex buffers
    // - element/index buffers
    // - VAOs
    
    const VertexV3V2 vertices[] = {
        {glm::vec3(-0.5, -0.5, 0.0), glm::vec2(0.0, 0.0)}, // bottom-left vertex
        {glm::vec3(0.5, -0.5, 0.0), glm::vec2(1.0, 0.0)}, // bottom-right vertex
        {glm::vec3(0.5,  0.5, 0.0), glm::vec2(1.0, 1.0)}, // top-right vertex
        {glm::vec3(-0.5,  0.5, 0.0), glm::vec2(0.0, 1.0)}, // top-left vertex
    };

    const u32 indices[] = {
        0, 1, 2,
        0, 2, 3
    };
    Model model = {};
    Mesh mesh = {};

    // Geometry (vertex buffer object & element buffer object, gpu side)
    glGenBuffers(1, &mesh.vertexBufferHandle);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vertexBufferHandle);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &mesh.indexBufferHandle);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.indexBufferHandle);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Attribute state, VAO is like an object to store client side, bind vbo with ebo, with shaders, etc...
    /*glGenVertexArrays(1, &app->vao);
    glBindVertexArray(app->vao);

    glBindBuffer(GL_ARRAY_BUFFER, app->embeddedVertices);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexV3V2), (void*)0); //layout (location = 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexV3V2), (void*)sizeof(glm::vec3)); //layout (location = 1)
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, app->embeddedElements);

    glBindVertexArray(0);
    */
    VertexBufferLayout vertexBufferLayout = {};
    vertexBufferLayout.attributes.push_back(VertexBufferAttribute{0, 3, 0}); // 3D positions
    vertexBufferLayout.attributes.push_back(VertexBufferAttribute{1, 2, sizeof(glm::vec3)}); // 2D tex coords
    vertexBufferLayout.stride = sizeof(VertexV3V2);
    
    SubMesh subMesh = {};
    subMesh.vertexBufferLayout = vertexBufferLayout;

    const u32 verticesSize = std::size(vertices);
    for (u32 i = 0; i < verticesSize; ++i)
    {
        std::vector<float> vec = vertices[i].GetVector();
        subMesh.vertices.insert(subMesh.vertices.end(), vec.begin(), vec.end());
    }
    
    subMesh.indices = std::vector<u32>(indices, indices + std::size(indices));
    mesh.subMeshes.push_back(subMesh);

    app->meshes.push_back(mesh);
    model.meshIdx = (u32)app->meshes.size() - 1;
    app->models.push_back(model);
    
    // - programs (and retrieve uniform indices)
    app->texturedGeometryProgramIdx = ShaderSupport::LoadProgram(app, "shaders.glsl", "TEXTURED_GEOMETRY");
    
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
        const u64 currentTimeStamp = GetFileLastWriteTimestamp(program.filepath.c_str());
        if (currentTimeStamp > program.lastWriteTimestamp)
        {
            glDeleteProgram(program.handle);
            const String programSource = ReadTextFile(program.filepath.c_str());
            const char* programName = program.programName.c_str();
            program.handle = ShaderSupport::CreateProgramFromSource(programSource, programName);
            program.lastWriteTimestamp = currentTimeStamp;
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




inline GLuint ShaderSupport::CreateProgramFromSource(String programSource, const char* shaderName)
{
        GLchar infoLogBuffer[1024] = {};
        GLsizei infoLogBufferSize = sizeof(infoLogBuffer);
        GLsizei infoLogSize;
        GLint success;

        char versionString[] = "#version 430\n";
        char shaderNameDefine[128];
        sprintf(shaderNameDefine, "#define %s\n", shaderName);
        char vertexShaderDefine[] = "#define VERTEX\n";
        char fragmentShaderDefine[] = "#define FRAGMENT\n";

        const GLchar* vertexShaderSource[] = {
            versionString,
            shaderNameDefine,
            vertexShaderDefine,
            programSource.str
        };
        const GLint vertexShaderLengths[] = {
            (GLint)strlen(versionString),
            (GLint)strlen(shaderNameDefine),
            (GLint)strlen(vertexShaderDefine),
            (GLint)programSource.len
        };
        const GLchar* fragmentShaderSource[] = {
            versionString,
            shaderNameDefine,
            fragmentShaderDefine,
            programSource.str
        };
        const GLint fragmentShaderLengths[] = {
            (GLint)strlen(versionString),
            (GLint)strlen(shaderNameDefine),
            (GLint)strlen(fragmentShaderDefine),
            (GLint)programSource.len
        };

        const GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vShader, std::size(vertexShaderSource), vertexShaderSource, vertexShaderLengths);
        glCompileShader(vShader);
        glGetShaderiv(vShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(vShader, infoLogBufferSize, &infoLogSize, infoLogBuffer);
            ELOG("glCompileShader() failed with vertex shader %s\nReported message:\n%s\n", shaderName, infoLogBuffer)
        }

        const GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fShader, std::size(fragmentShaderSource), fragmentShaderSource, fragmentShaderLengths);
        glCompileShader(fShader);
        glGetShaderiv(fShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(fShader, infoLogBufferSize, &infoLogSize, infoLogBuffer);
            ELOG("glCompileShader() failed with fragment shader %s\nReported message:\n%s\n", shaderName,
                 infoLogBuffer)
        }

        const GLuint programHandle = glCreateProgram();
        glAttachShader(programHandle, vShader);
        glAttachShader(programHandle, fShader);
        glLinkProgram(programHandle);
        glGetProgramiv(programHandle, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(programHandle, infoLogBufferSize, &infoLogSize, infoLogBuffer);
            ELOG("glLinkProgram() failed with program %s\nReported message:\n%s\n", shaderName, infoLogBuffer)
        }

        glUseProgram(0);

        glDetachShader(programHandle, vShader);
        glDetachShader(programHandle, fShader);
        glDeleteShader(vShader);
        glDeleteShader(fShader);

        return programHandle;
}

inline u32 ShaderSupport::LoadProgram(App* app, const char* filepath, const char* programName)
{
    const String programSource = ReadTextFile(filepath);
    Program program = {};
    program.handle = CreateProgramFromSource(programSource, programName);
    program.filepath = filepath;
    program.programName = programName;
    program.lastWriteTimestamp = GetFileLastWriteTimestamp(filepath);
    app->programs.push_back(program);

    return app->programs.size() - 1;
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

Image TextureSupport::LoadImage(const char* filename)
{
    Image img = {};
    stbi_set_flip_vertically_on_load(true);
    img.pixels = stbi_load(filename, &img.size.x, &img.size.y, &img.nchannels, 0);
    if (img.pixels)
    {
        img.stride = img.size.x * img.nchannels;
    }
    else
    {
        ELOG("Could not open file %s", filename)
    }
    return img;
}

void TextureSupport::FreeImage(const Image& image)
{
    stbi_image_free(image.pixels);
}

GLuint TextureSupport::CreateTexture2DFromImage(const Image& image)
{
    GLenum internalFormat = GL_RGB8;
    GLenum dataFormat     = GL_RGB;
    const GLenum dataType       = GL_UNSIGNED_BYTE;

    switch (image.nchannels)
    {
    case 3: dataFormat = GL_RGB; internalFormat = GL_RGB8; break;
    case 4: dataFormat = GL_RGBA; internalFormat = GL_RGBA8; break;
    default: ELOG("LoadTexture2D() - Unsupported number of channels")
    }

    GLuint texHandle;
    glGenTextures(1, &texHandle);
    glBindTexture(GL_TEXTURE_2D, texHandle);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, image.size.x, image.size.y, 0, dataFormat, dataType, image.pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    return texHandle;
}

u32 TextureSupport::LoadTexture2D(App* app, const char* filepath)
{
    for (u32 texIdx = 0; texIdx < app->textures.size(); ++texIdx)
        if (app->textures[texIdx].filepath == filepath)
            return texIdx;

    Image image = LoadImage(filepath);

    if (image.pixels)
    {
        Texture tex = {};
        tex.handle = CreateTexture2DFromImage(image);
        tex.filepath = filepath;

        const u32 texIdx = (u32)app->textures.size();
        app->textures.push_back(tex);

        FreeImage(image);
        return texIdx;
    }
    else
    {
        return UINT32_MAX;
    }
}