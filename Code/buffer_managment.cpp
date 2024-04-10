#include "buffer_management.h"
#define _CRT_SECURE_NO_WARNINGS

GLint BufferManagement::maxUniformBufferSize = 0;
GLint BufferManagement::uniformBlockAlignment = 0;

bool BufferManagement::IsPowerOf2(const u32 value)
{
    return value && !(value & (value - 1));
}
bool BufferManagement::IsMultipleOf(const u32 value, const u32 divider)
{
    return (value % divider) == 0;
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
void BufferManagement::SetBufferBlockStart(Buffer& buffer, const u32 alignment, u32& offset)
{
    AlignHead(buffer, alignment);
    offset = buffer.head;
}
void BufferManagement::SetBufferBlockEnd(Buffer& buffer, const u32 alignment, u32& size, const u32& offset)
{
    AlignHead(buffer, alignment);
    size = buffer.head - offset;
}
void BufferManagement::InitUniformBuffer()
{
    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &maxUniformBufferSize);
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &uniformBlockAlignment);
}
void BufferManagement::DeleteBuffer(const Buffer& buffer)
{
    glDeleteBuffers(1, &buffer.handle);
}

void BufferManagement::BindBufferRange(const Buffer& buffer, const u32 bindingPoint = 0, const u32 blockSize = 0, const u32 blockOffset = 0)
{
    ASSERT(IsMultipleOf(blockSize, BufferManagement::uniformBlockAlignment), "The size must be multiple of uniform block alignment");
    glBindBufferRange(GL_UNIFORM_BUFFER, bindingPoint, buffer.handle, blockOffset, blockSize);
}

Buffer FrameBufferManagement::CreateFrameBuffer()
{
    Buffer buffer = {};
    buffer.type = GL_FRAMEBUFFER;

    glGenFramebuffers(1, &buffer.handle);
    
    return buffer;
}

void FrameBufferManagement::DeleteFrameBuffer(const Buffer& buffer)
{
    glDeleteFramebuffers(1, &buffer.handle);
}
void FrameBufferManagement::BindFrameBuffer(const Buffer& buffer)
{
    glBindFramebuffer(GL_FRAMEBUFFER, buffer.handle);
}
void FrameBufferManagement::UnBindFrameBuffer(const Buffer& buffer)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBufferManagement::SetColorAttachment(const Buffer& buffer, const GLint colorTextureIdx, const GLuint layoutLocation)
{
    glFramebufferTexture2D(GL_FRAMEBUFFER, static_cast<GLenum>(GL_COLOR_ATTACHMENT0 + layoutLocation), GL_TEXTURE_2D, colorTextureIdx, 0);
}
void FrameBufferManagement::SetDepthAttachment(const Buffer& buffer, const GLint depthTextureIdx)
{
    glFramebufferTexture2D(GL_FRAMEBUFFER, static_cast<GLenum>(GL_DEPTH_ATTACHMENT), GL_TEXTURE_2D, depthTextureIdx, 0);
}
void FrameBufferManagement::SetDrawBuffersTextures(const std::vector<u32>& activeAttachments)
{
    std::vector<GLenum> buffers(activeAttachments.size());
    
    for (u32 i = 0; i < buffers.size(); ++i)
        buffers[i] = (GLenum)(GL_COLOR_ATTACHMENT0 + activeAttachments[i]);

    glDrawBuffers(static_cast<GLsizei>(buffers.size()), buffers.data());
}
bool FrameBufferManagement::CheckStatus()
{
    bool ret = true;
    const GLenum frameBufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (frameBufferStatus != GL_FRAMEBUFFER_COMPLETE)
    {
        ret = false;
        switch (frameBufferStatus)
        {
            // If specified framebuffer is the default read or draw framebuffer, but the default framebuffer does not exist
        case GL_FRAMEBUFFER_UNDEFINED: ELOG("GL_FRAMEBUFFER_UNDEFINED") break;

            // If any of the framebuffer attachment points are framebuffer incomplete
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT : ELOG("GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT") break;

            // If the framebuffer does not have at least one image attached to it
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT  : ELOG("GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT") break;

            // If the value of GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE is GL_NONE for any color attachment point(s) named by GL_DRAW_BUFFERi
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER    : ELOG("GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER  ") break;

            // If GL_READ_BUFFER is not GL_NONE and the value of GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE is GL_NONE for the color attachment point named by GL_READ_BUFFER
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER   : ELOG("GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER ") break;

            // If the combination of internal formats of the attached images violates an implementation-dependent set of restrictions
        case GL_FRAMEBUFFER_UNSUPPORTED   : ELOG("GL_FRAMEBUFFER_UNSUPPORTED ") break;

            // Check: https://registry.khronos.org/OpenGL-Refpages/gl4/html/glCheckFramebufferStatus.xhtml
        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE   : ELOG("GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT") break;

            // If any framebuffer attachment is layered, and any populated attachment is not layered, or if all populated color attachments are not from textures of the same target
        case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS   : ELOG("GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS ") break;
        default: {
            if (frameBufferStatus == GL_INVALID_ENUM )
                ELOG("Target is not GL_DRAW_FRAMEBUFFER, GL_READ_FRAMEBUFFER or GL_FRAMEBUFFER ")
            else if (frameBufferStatus == GL_INVALID_OPERATION )
                ELOG("Framebuffer is not zero or the name of an existing framebuffer object ")
            else
                ELOG("Unkown framebuffer status error")
        }
        }
    }
    return ret;
}
