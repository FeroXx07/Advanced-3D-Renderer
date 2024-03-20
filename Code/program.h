#ifndef PROGRAM_H
#define PROGRAM_H
#include "platform.h"
#include "vertex.h"

struct Program
{
    GLuint             handle;
    std::vector<std::string>    filepaths;
    std::string        programName;
    u64                lastWriteTimestamp; // What is this for?
    VertexShaderLayout  vertexInputLayout;
};
#endif // PROGRAM_H
