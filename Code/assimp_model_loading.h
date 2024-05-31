#ifndef ASSIMP_MODEL_LOADING_H
#define ASSIMP_MODEL_LOADING_H
#include <vector>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "platform.h"

struct Material;
struct Mesh;
struct App;

struct AssimpSupport
{
    static u32 LoadModel(App* app, const char* filename);
    static void ProcessAssimpNode(const aiScene* scene, const aiNode *node, Mesh *myMesh, u32 baseMeshMaterialIndex, std::vector<u32>& submeshMaterialIndices);
    static void ProcessAssimpMaterial(App* app, const aiMaterial *material, Material& myMaterial, const std::string& directory);
    static void ProcessAssimpMesh(const aiScene* scene, aiMesh *mesh, Mesh *myMesh, u32 baseMeshMaterialIndex, std::vector<u32>& submeshMaterialIndices);
};

#endif // ASSIMP_MODEL_LOADING_H
