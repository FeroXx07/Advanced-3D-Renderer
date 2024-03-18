#ifndef PROGRAM_H
#define PROGRAM_H
#include "platform.h"
#include "vertex.h"

struct Program
{
    GLuint             handle;
    std::string        filepath;
    std::string        programName;
    u64                lastWriteTimestamp; // What is this for?
    VertexShaderLayout  vertexInputLayout;
};
#endif // PROGRAM_H
