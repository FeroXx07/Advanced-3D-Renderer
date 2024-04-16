#ifndef MESH_H
#define MESH_H
#include "vertex.h"
#include <vector>

#include "buffer_management.h"
#include "program.h"

struct Model
{
    Model(const char* name) : name(name), meshIdx(0)
    {
    }

    std::string name;
    u32 meshIdx;
    std::vector<u32> materialIdx;
};

struct Material
{
    std::string name;
    vec3 albedo;
    vec3 emissive;
    f32 smoothness;
    u32 albedoTextureIdx;
    u32 emissiveTextureIdx;
    u32 specularTextureIdx;
    u32 normalsTextureIdx;
    u32 bumpTextureIdx;

    u32 paramsOffset;
    u32 paramsSize;
};

struct SubMesh
{
    SubMesh(const char* name) : name(name), vertexBufferLayout(), vertexOffset(0), indexOffset(0)
    {
    }

    std::string name;
    VertexBufferLayout vertexBufferLayout;
    std::vector<float> vertices;
    std::vector<u32> indices;
    u32 vertexOffset;
    u32 indexOffset;

    std::vector<VAO> vaoList;
};

struct Mesh
{
    Mesh(const char* name) : name(name)
    {
    }

    std::string name;
    std::vector<SubMesh> subMeshes;
    Buffer vertexBuffer;
    Buffer indexBuffer;

    void DrawSubMesh(u32 subMeshIndex, const Texture& texture, const u32 textureUniform, const Program& program, const bool drawWireFrame = false);
    void DrawSubMesh(u32 subMeshIndex, const std::vector<u32>& textureUniformsHandles, const std::vector<u32>& textureUniformsLocations, const Program& program, const bool drawWireFrame = false);
};


struct VAOSupport
{
    static void CreateNewVAO(const Mesh& mesh, const SubMesh& subMesh, const Program& program, GLuint& vaoHandle);
    static GLuint FindVAO(Mesh& mesh, const u32 subMeshIndex, const Program& program);
};

inline void Mesh::DrawSubMesh(u32 subMeshIndex, const Texture& texture, const u32 textureUniform, const Program& program, const bool drawWireFrame)
{
    const SubMesh& subMesh = subMeshes[subMeshIndex];

    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 1, -1, subMesh.name.c_str());
    const GLuint vao = VAOSupport::FindVAO(*this, subMeshIndex, program);
                
    if (drawWireFrame)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                
    glBindVertexArray(vao);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture.handle);
    glUniform1i(static_cast<GLint>(textureUniform), 0); // stackoverflow.com/questions/23687102/gluniform1f-vs-gluniform1i-confusion

    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(subMesh.indices.size()), GL_UNSIGNED_INT, reinterpret_cast<void*>(static_cast<u64>(subMesh.indexOffset)));
    glPopDebugGroup();
}
inline void Mesh::DrawSubMesh(u32 subMeshIndex, const std::vector<u32>& textureUniformsHandles, const std::vector<u32>& textureUniformsLocations, const Program& program, const bool drawWireFrame)
{
    const SubMesh& subMesh = subMeshes[subMeshIndex];

    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 1, -1, subMesh.name.c_str());
    const GLuint vao = VAOSupport::FindVAO(*this, subMeshIndex, program);
                
    if (drawWireFrame)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    
    assert(textureUniformsHandles.size() == textureUniformsLocations.size());
    
    for (u32 i = 0; i < textureUniformsLocations.size(); ++i)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, textureUniformsHandles[i]);
        //glUniform1i(static_cast<GLint>(textureUniformsLocations[i]), static_cast<GLint>(i)); // stackoverflow.com/questions/23687102/gluniform1f-vs-gluniform1i-confusion
    }
    
    glBindVertexArray(vao);

    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(subMesh.indices.size()), GL_UNSIGNED_INT, reinterpret_cast<void*>(static_cast<u64>(subMesh.indexOffset)));
    glPopDebugGroup();
}

#endif // MESH_H
