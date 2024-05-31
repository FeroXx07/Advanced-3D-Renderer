#include "program.h"
#include "app.h"
#include <iostream>

GLuint ShaderSupport::CreateProgramFromSource(std::string programSource, const char* shaderName)
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
        programSource.c_str()
    };
    const GLint vertexShaderLengths[] = {
        (GLint)strlen(versionString),
        (GLint)strlen(shaderNameDefine),
        (GLint)strlen(vertexShaderDefine),
        (GLint)programSource.size()
    };
    const GLchar* fragmentShaderSource[] = {
        versionString,
        shaderNameDefine,
        fragmentShaderDefine,
        programSource.c_str()
    };
    const GLint fragmentShaderLengths[] = {
        (GLint)strlen(versionString),
        (GLint)strlen(shaderNameDefine),
        (GLint)strlen(fragmentShaderDefine),
        (GLint)programSource.size()
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

GLuint ShaderSupport::CreateProgramFromSource(const std::string& shaderSourceVert, const std::string& shaderSourceFrag, const char* shaderName)
{
    GLchar infoLogBuffer[1024] = {};
    GLsizei infoLogBufferSize = sizeof(infoLogBuffer);
    GLsizei infoLogSize;
    GLint success;
    const char* vShaderSource = shaderSourceVert.c_str();
    const char* fShaderSource = shaderSourceFrag.c_str();
    
    const GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vShader, 1, &vShaderSource, NULL);
    glCompileShader(vShader);
    glGetShaderiv(vShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vShader, infoLogBufferSize, &infoLogSize, infoLogBuffer);
        ELOG("glCompileShader() failed with vertex shader %s\nReported message:\n%s\n", shaderName, infoLogBuffer)
    }

    const GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fShader, 1, &fShaderSource, NULL);
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

u32 ShaderSupport::LoadProgram(App* app, const char* filepath, const char* programName)
{
    const std::string programSource = ReadTextFile(filepath);
    Program program = {};
    program.handle = CreateProgramFromSource(programSource.c_str(), programName);
    program.filePaths.emplace_back(filepath);
    program.programName = programName;
    program.lastWriteTimestamp = GetFileLastWriteTimestamp(filepath);
    app->programs.push_back(program);

    return app->programs.size() - 1;
}

u32 ShaderSupport::LoadProgram(App* app, const char* filepathVert, const char* filepathFrag, const char* programName)
{
    const std::string programSourceVert = ReadTextFile(filepathVert);
    const std::string programSourceFrag = ReadTextFile(filepathFrag);
    Program program = {};
    program.handle = CreateProgramFromSource(programSourceVert.c_str(), programSourceFrag.c_str(), programName);
    program.filePaths.emplace_back(filepathVert);
    program.filePaths.emplace_back(filepathFrag);
    program.programName = programName;
    program.lastWriteTimestamp = GetFileLastWriteTimestamp(filepathVert);
    app->programs.push_back(program);

    return app->programs.size() - 1;
}