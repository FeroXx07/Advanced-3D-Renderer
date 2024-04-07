//
// platform.h : This file contains basic platform types and tools. Also, it exposes
// the necessary functions for the Engine to communicate with the Platform layer.
//

#pragma once

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>

#include "glad/glad.h"

#pragma warning(disable : 4267) // conversion from X to Y, possible loss of data

typedef char                   i8;
typedef short                  i16;
typedef int                    i32;
typedef long long int          i64;
typedef unsigned char          u8; // 0...255
typedef unsigned short         u16;
typedef unsigned int           u32;
typedef unsigned long long int u64;
typedef float                  f32;
typedef double                 f64;

typedef glm::vec2  vec2;
typedef glm::vec3  vec3;
typedef glm::vec4  vec4;
typedef glm::ivec2 ivec2;
typedef glm::ivec3 ivec3;
typedef glm::ivec4 ivec4;

enum MouseButton {
    LEFT,
    RIGHT,
    MOUSE_BUTTON_COUNT
};

enum Key {
    K_SPACE,
    K_0, K_1, K_2, K_3, K_4, K_5, K_6, K_7, K_8, K_9,
    K_A, K_B, K_C, K_D, K_E, K_F, K_G, K_H, K_I, K_J, K_K, K_L, K_M,
    K_N, K_O, K_P, K_Q, K_R, K_S, K_T, K_U, K_V, K_W, K_X, K_Y, K_Z,
    K_ENTER, K_ESCAPE,
    KEY_COUNT
};

enum ButtonState {
    BUTTON_IDLE,
    BUTTON_PRESS,
    BUTTON_PRESSED,
    BUTTON_RELEASE
};

struct Input {
    glm::vec2   mousePos;
    glm::vec2   mouseDelta;
    ButtonState mouseButtons[MOUSE_BUTTON_COUNT];
    ButtonState keys[KEY_COUNT];
};

struct String
{
    char* str;
    u32   len;
};

String MakeString(const char *cstr);

String MakePath(String dir, String filename);

String GetDirectoryPart(String path);

/**
 * Reads a whole file and returns a string with its contents. The returned string
 * is temporary and should be copied if it needs to persist for several frames.
 */
String ReadTextFile(const char *filepath);

/**
 * It retrieves a timestamp indicating the last time the file was modified.
 * Can be useful in order to check for file modifications to implement hot reloads.
 */
u64 GetFileLastWriteTimestamp(const char *filepath);

/**
 * It logs a string to whichever outputs are configured in the platform layer.
 * By default, the string is printed in the output console of VisualStudio.
 */
void LogString(const char* str);

#define ILOG(...)                 \
{                                 \
char logBuffer[1024] = {};        \
sprintf_s(logBuffer, __VA_ARGS__);  \
LogString(logBuffer);             \
}

#define ELOG(...) ILOG(__VA_ARGS__)

#define ARRAY_COUNT(array) (sizeof(array)/sizeof(array[0]))

#define ASSERT(condition, message) assert((condition) && message)

#define KB(count) (1024*(count))
#define MB(count) (1024*KB(count))
#define GB(count) (1024*MB(count))

#define PI  3.14159265359f
#define TAU 6.28318530718f

struct OpenGlContext
{
    char versionO[64];
    std::string version;
    std::string renderer;
    std::string vendor;
    std::string glslVersion;
};

OpenGlContext RetrieveOpenGLContext();

inline std::string convertOpenGLDataTypeToString(u32 dataType) {
    switch(dataType) {
    case 5126: return "Float";
    case 35674: return "FloatMat2";
    case 35685: return "FloatMat2x3";
    case 35686: return "FloatMat2x4";
    case 35675: return "FloatMat3";
    case 35687: return "FloatMat3x2";
    case 35688: return "FloatMat3x4";
    case 35676: return "FloatMat4";
    case 35689: return "FloatMat4x2";
    case 35690: return "FloatMat4x3";
    case 35664: return "FloatVec2";
    case 35665: return "FloatVec3";
    case 35666: return "FloatVec4";
    case 5124: return "Int";
    case 35667: return "IntVec2";
    case 35668: return "IntVec3";
    case 35669: return "IntVec4";
    case 5125: return "UnsignedInt";
    case 36294: return "UnsignedIntVec2";
    case 36295: return "UnsignedIntVec3";
    case 36296: return "UnsignedIntVec4";
    default: return "Unknown";
    }
}