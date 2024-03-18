#ifndef MESH_H
#define MESH_H
#include "vertex.h"
#include <vector>

struct Model
{
    u32 meshIdx;
    std::vector<u32> materialIdx;
};
struct SubMesh
{
    VertexBufferLayout vertexBufferLayout;
    std::vector<float> vertices;
    std::vector<u32> indices;
    u32 vertexOffset;
    u32 indexOffset;

    std::vector<VAO> vaoList;
};

struct Mesh
{
    std::vector<SubMesh> subMeshes;
    GLuint vertexBufferHandle;
    GLuint indexBufferHandle;
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
