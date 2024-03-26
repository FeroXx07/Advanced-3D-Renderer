#ifndef MESH_H
#define MESH_H
#include "vertex.h"
#include <vector>

#include "buffer_management.h"

struct Model
{
    Model(const char* name) : name(name), meshIdx(0)
    {
    }

    std::string name;
    u32 meshIdx;
    std::vector<u32> materialIdx;
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
};
#endif // MESH_H
