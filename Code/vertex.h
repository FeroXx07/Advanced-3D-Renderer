#ifndef VERTEX_H
#define VERTEX_H
#include "platform.h"

/*
 *  Documentation on GLSL
 *  www.khronos.org/opengl/wiki/Layout_Qualifier_(GLSL)
 *  www.khronos.org/opengl/wiki/Uniform_(GLSL)
 */

struct VertexV3V2
{
    glm::vec3 pos;
    glm::vec2 uv;

    std::vector<float> GetVector() const
    {
        std::vector<float> array(5);
    
        // Populate array with data
        // For example:
        array[0] = pos.x;
        array[1] = pos.y;
        array[2] = pos.z;
        array[3] = uv.x;
        array[4] = uv.y;

        return array;
    }
};

/// <summary>
/// Attribute of a VBO
/// </summary>
/// <param name="location">Layout qualifier in GLSL.</param>
/// <param name="componentCount">Num of components.</param>
/// <param name="offset">Offset inside the array stride.</param>
struct VertexBufferAttribute
{
    u8 location;
    u8 componentCount;
    u8 offset;
};

/// <summary>
/// Layout of a VBO
/// </summary>
/// <param name="attributes">List of attributes.</param>
/// <param name="stride">Stride size of the array.</param>
struct VertexBufferLayout
{
    std::vector<VertexBufferAttribute> attributes;
    u8 stride; // Stride in this struct and not VertexBufferAttribute because it is the same for the whole (VBO) VertexBufferLayout
};

/// <summary>
/// Attribute of a shader
/// </summary>
/// <param name="location">Layout qualifier in GLSL.</param>
/// <param name="componentCount">Num of components.</param>
struct VertexShaderAttribute
{
    u8 location;
    u8 componentCount;
};

/// <summary>
/// Layout of a shader
/// </summary>
/// <param name="attributes">List of attributes.</param>
struct VertexShaderLayout
{
    std::vector<VertexShaderAttribute> attributes;
};

/// <summary>
/// Object that bind together the VBO, EBO, shader and other state
/// </summary>
/// <param name="handle">Buffer handle returned from OpenGL.</param>
/// <param name="programHandle">Shader handle returned from OpenGL.</param>
struct VAO
{
    GLuint handle;
    GLuint programHandle;
};

#endif // VERTEX_H
