//
// engine.h: This file contains the types and functions relative to the engine.
//

#pragma once

#include "app.h"

void Init(App* app);

void Gui(const App* app);

void Update(App* app);

void Render(App* app);

struct TextureSupport
{
    static Image LoadImage(const char* filename);
    static void FreeImage(const Image& image);
    static GLuint CreateTexture2DFromImage(const Image& image);
    static u32 LoadTexture2D(App* app, const char* filepath);
    
};

struct ShaderSupport
{
    static GLuint CreateProgramFromSource(std::string programSource, const char* shaderName);
    static GLuint CreateProgramFromSource(const std::string& shaderSourceVert, const std::string& shaderSourceFrag, const char* shaderName);
    static u32 LoadProgram(App* app, const char* filepath, const char* programName);
    static u32 LoadProgram(App* app, const char* filepathVert, const char* filepathFrag, const char* programName);

};

struct VAOSupport
{
    static void CreateNewVAO(const Mesh& mesh, const SubMesh& subMesh, const Program& program, GLuint& vaoHandle);
    static GLuint FindVAO(Mesh& mesh, const u32 subMeshIndex, const Program& program);
};


