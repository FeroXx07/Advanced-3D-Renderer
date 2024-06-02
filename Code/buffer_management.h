#ifndef BUFFER_MANAGEMENT_H
#define BUFFER_MANAGEMENT_H
#include <vector>

#include "platform.h"
#include "texture.h"

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
    // Helpers for assertions
    static bool IsPowerOf2(const u32 value);
    static bool IsMultipleOf(const u32 value, const u32 divider);
    
    // Rounds down the adjusted value to the nearest multiple of the alignment boundary
    static u32 Align(u32 value, u32 alignment);
    static Buffer CreateBuffer(const u32 size, const GLenum type, const GLenum usage, void* data);

    // Binding and unbinding of buffers
    static void BindBuffer(const Buffer& buffer);
    static void UnBindBuffer(const Buffer& buffer);
    static void BindBufferRange(const Buffer& buffer, const u32 bindingPoint, const u32 blockSize, const u32 blockOffset);

    // Mapping and unmapping of buffers for data insertion
    static void MapBuffer(Buffer& buffer, GLenum access);
    static void UnmapBuffer(const Buffer& buffer);
    static void AlignHead(Buffer& buffer, u32 alignment);
    static void PushAlignedData(Buffer& buffer, const void* data, u32 size, u32 alignment);

    // Alignment of buffer blocks
    static void SetBufferBlockStart(Buffer& buffer, const u32 alignment, u32& offset);
    static void SetBufferBlockEnd(Buffer& buffer, const u32 alignment, u32& size, const u32& offset);

    // Init and destroy of buffers
    static void InitUniformBuffer();
    static void DeleteBuffer(const Buffer& buffer);

    // Global info on GLSL config
    static GLint maxUniformBufferSize;
    static GLint uniformBlockAlignment;
};

class FrameBufferManagement
{
public:
    static Buffer CreateFrameBuffer();
    static void DeleteFrameBuffer(const Buffer& buffer);

    static void BindFrameBuffer(const Buffer& buffer);
    static void UnBindFrameBuffer(const Buffer& buffer);

    static void SetColorAttachment(const Buffer& buffer, const GLint colorTextureIdx, const GLuint layoutLocation);
    static void SetDepthAttachment(const Buffer& buffer, const GLint depthTextureIdx);

    static void SetDrawBuffersTextures(const std::vector<u32>& activeAttachments);

    static bool CheckStatus();
};

#define CREATE_CONSTANT_BUFFER(size, data) BufferManagement::CreateBuffer(size, GL_UNIFORM_BUFFER, GL_STREAM_DRAW, data)
#define CREATE_STATIC_VERTEX_BUFFER(size, data) BufferManagement::CreateBuffer(size, GL_ARRAY_BUFFER, GL_STATIC_DRAW, data)
#define CREATE_STATIC_INDEX_BUFFER(size, data) BufferManagement::CreateBuffer(size, GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, data)

// sizeof(vec4) is the 4N where, N = 4 bytes is the basic machine unit,
#define PUSH_DATA(buffer, data, size) BufferManagement::PushAlignedData(buffer, data, size, 1)
#define PUSH_U_INT(buffer, value) { u32 v = value; BufferManagement::PushAlignedData(buffer, &v, sizeof(v), BASIC_MACHINE_UNIT); }
#define PUSH_VEC2(buffer, value) BufferManagement::PushAlignedData(buffer, value_ptr(value), sizeof(value), 2 * BASIC_MACHINE_UNIT)
#define PUSH_VEC3(buffer, value) BufferManagement::PushAlignedData(buffer, value_ptr(value), sizeof(value), 4 * BASIC_MACHINE_UNIT)
#define PUSH_VEC4(buffer, value) BufferManagement::PushAlignedData(buffer, value_ptr(value), sizeof(value), 4 * BASIC_MACHINE_UNIT)
#define PUSH_MAT3(buffer, value) BufferManagement::PushAlignedData(buffer, value_ptr(value), sizeof(value), 4 * BASIC_MACHINE_UNIT)
#define PUSH_MAT4(buffer, value) BufferManagement::PushAlignedData(buffer, value_ptr(value), sizeof(value), 4 * BASIC_MACHINE_UNIT)
#define PUSH_FLOAT(buffer, value) { f32 v = value; BufferManagement::PushAlignedData(buffer, &v, sizeof(v), BASIC_MACHINE_UNIT); }

enum DRAW_BUFFERS_LOCATION
{
    RT_LOCATION_COLOR = 0,
    RT_LOCATION_POSITION_WORLD_SPACE = 1,
    RT_LOCATION_NORMAL = 2,
    RT_LOCATION_SPECULAR_ROUGHNESS = 3,
    RT_LOCATION_SSAO = 4,
    RT_LOCATION_BUMP = 5,
    RT_LOCATION_TANGENT = 6,
    RT_LOCATION_FINAL_RESULT = 7
};

enum MAT_TEXTURE_LOCATION
{
    MAT_T_DIFFUSE = 0,
    MAT_T_NORMALS = 1,
    MAT_T_SPECULAR = 2,
    MAT_T_BUMP = 3
};

enum VERTEX_ATTRIBUTE_LOCATION
{
    ATTR_LOCATION_POSITION = 0,
    ATTR_LOCATION_NORMAL = 1,
    ATTR_LOCATION_TEXTCOORD = 2,
    ATTR_LOCATION_TANGENT = 3,
    ATTR_LOCATION_BITANGENT = 4,
};

enum STD_140_BINDING_POINT
{
    BP_GLOBAL_PARAMS = 0,
    BP_LOCAL_PARAMS = 1,
    BP_MATERIAL_PARAMS = 2,
    BP_SSAO_PARAMS = 3
};

#endif // BUFFER_MANAGEMENT_H
