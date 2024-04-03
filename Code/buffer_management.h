#ifndef BUFFER_MANAGEMENT_H
#define BUFFER_MANAGEMENT_H
#include "platform.h"

#define BASIC_MACHINE_UNIT 4

struct Buffer
{
    Buffer(): size(0), type(0), handle(0), data(nullptr), head(0)
    {
    }

    u32 size;
    GLenum type;
    GLuint handle;
    
    void* data;
    u32 head;
};

class BufferManagement
{
public:
    static bool IsPowerOf2(const u32 value);
    static bool IsMultipleOf(const u32 value, const u32 divider);
    
    // Rounds down the adjusted value to the nearest multiple of the alignment boundary
    static u32 Align(u32 value, u32 alignment);
    static Buffer CreateBuffer(const u32 size, const GLenum type, const GLenum usage, void* data);
    
    static void BindBuffer(const Buffer& buffer);
    static void UnBindBuffer(const Buffer& buffer);
    
    static void MapBuffer(Buffer& buffer, GLenum access);
    static void UnmapBuffer(const Buffer& buffer);
    static void AlignHead(Buffer& buffer, u32 alignment);
    static void PushAlignedData(Buffer& buffer, const void* data, u32 size, u32 alignment);
    static void InitUniformBuffer();

    static void BindBufferRange(const Buffer& buffer, const u32 bindingPoint, const u32 blockSize, const u32 blockOffset);
    
    static GLint maxUniformBufferSize;
    static GLint uniformBlockAlignment;
};

#define CREATE_CONSTANT_BUFFER(size, data) BufferManagement::CreateBuffer(size, GL_UNIFORM_BUFFER, GL_STREAM_DRAW, data)
#define CREATE_STATIC_VERTEX_BUFFER(size, data) BufferManagement::CreateBuffer(size, GL_ARRAY_BUFFER, GL_STATIC_DRAW, data)
#define CREATE_STATIC_INDEX_BUFFER(size, data) BufferManagement::CreateBuffer(size, GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, data)

// sizeof(vec4) is the 4N where, N = 4 bytes is the basic machine unit,
#define PUSH_DATA(buffer, data, size) BufferManagement::PushAlignedData(buffer, data, size, 1)
#define PUSH_U_INT(buffer, value) { u32 v = value; BufferManagement::PushAlignedData(buffer, &v, sizeof(v), BASIC_MACHINE_UNIT); }
#define PUSH_VEC3(buffer, value) BufferManagement::PushAlignedData(buffer, value_ptr(value), sizeof(value), 4 * BASIC_MACHINE_UNIT)
#define PUSH_VEC4(buffer, value) BufferManagement::PushAlignedData(buffer, value_ptr(value), sizeof(value), 4 * BASIC_MACHINE_UNIT)
#define PUSH_MAT3(buffer, value) BufferManagement::PushAlignedData(buffer, value_ptr(value), sizeof(value), 4 * BASIC_MACHINE_UNIT)
#define PUSH_MAT4(buffer, value) BufferManagement::PushAlignedData(buffer, value_ptr(value), sizeof(value), 4 * BASIC_MACHINE_UNIT)

#endif // BUFFER_MANAGEMENT_H
