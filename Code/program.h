#ifndef PROGRAM_H
#define PROGRAM_H
#define _CRT_SECURE_NO_WARNINGS
#include <vector>

#include "platform.h"
#include "vertex.h"

struct App;

struct Program
{
    GLuint             handle;
    std::vector<std::string>    filePaths;
    std::string        programName;
    u64                lastWriteTimestamp; // What is this for?
    VertexShaderLayout  vertexInputLayout;
};

struct ShaderSupport
{
    static GLuint CreateProgramFromSource(std::string programSource, const char* shaderName);
    static GLuint CreateProgramFromSource(const std::string& shaderSourceVert, const std::string& shaderSourceFrag, const char* shaderName);
    static u32 LoadProgram(App* app, const char* filepath, const char* programName);
    static u32 LoadProgram(App* app, const char* filepathVert, const char* filepathFrag, const char* programName);
};
#endif // PROGRAM_H
