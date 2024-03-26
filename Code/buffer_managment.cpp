#include "buffer_management.h"

GLint BufferManagement::maxUniformBufferSize = 0;
GLint BufferManagement::uniformBlockAlignment = 0;

bool BufferManagement::IsPowerOf2(const u32 value)
{
    return value && !(value & (value - 1));
}

u32 BufferManagement::Align(const u32 value, const u32 alignment)
{
    return (value + alignment - 1) & ~(alignment - 1);
}

Buffer BufferManagement::CreateBuffer(const u32 size, const GLenum type, const GLenum usage, void* data)
{
    Buffer buffer = {};
    buffer.size = size;
    buffer.type = type;

    glGenBuffers(1, &buffer.handle);
    glBindBuffer(type, buffer.handle);
    if (data != nullptr)
        glBufferData(type, buffer.size, data, usage);
    else
        glBufferData(type, buffer.size, NULL, usage);
    glBindBuffer(type, 0);

    return buffer;
}

void BufferManagement::BindBuffer(const Buffer& buffer)
{
    glBindBuffer(buffer.type, buffer.handle);
}
void BufferManagement::UnBindBuffer(const Buffer& buffer)
{
    glBindBuffer(buffer.type, 0);
}

void BufferManagement::MapBuffer(Buffer& buffer, const GLenum access)
{
    glBindBuffer(buffer.type, buffer.handle);
    buffer.data = (u8*)glMapBuffer(buffer.type, access);
    buffer.head = 0;
}

void BufferManagement::UnmapBuffer(const Buffer& buffer)
{
    glUnmapBuffer(buffer.type);
    glBindBuffer(buffer.type, 0);
}

void BufferManagement::AlignHead(Buffer& buffer, const u32 alignment)
{
    ASSERT(IsPowerOf2(alignment), "The alignment must be a power of 2");
    buffer.head = Align(buffer.head, alignment);
}

void BufferManagement::PushAlignedData(Buffer& buffer, const void* data, const u32 size, const u32 alignment)
{
    ASSERT(buffer.data != NULL, "The buffer must be mapped first");
    AlignHead(buffer, alignment);
    memcpy((u8*)buffer.data + buffer.head, data, size);
    buffer.head += size;
}
void BufferManagement::InitUniformBuffer()
{
    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &maxUniformBufferSize);
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &uniformBlockAlignment);
}
void BufferManagement::BindBufferRange(const Buffer& buffer, const u32 bindingPoint = 0, const u32 blockSize = 0, const u32 blockOffset = 0)
{
    glBindBufferRange(GL_UNIFORM_BUFFER, bindingPoint, buffer.handle, blockOffset, blockSize);
}
