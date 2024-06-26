﻿#define _CRT_SECURE_NO_WARNINGS
#include "assimp_model_loading.h"

#include "app.h"
#include "engine.h"
#include <iostream>
#include <filesystem> 

u32 AssimpSupport::LoadModel(App* app, const char* filename)
{
    // Define import flags
    const aiScene* scene = aiImportFile(filename,
                                        aiProcess_Triangulate           |
                                        aiProcess_GenSmoothNormals      |
                                        aiProcess_CalcTangentSpace      |
                                        aiProcess_JoinIdenticalVertices |
                                        aiProcess_PreTransformVertices  |
                                        aiProcess_ImproveCacheLocality  |
                                        aiProcess_OptimizeMeshes        |
                                        aiProcess_SortByPType);

    if (!scene)
    {
        ELOG("Error loading mesh %s: %s", filename, aiGetErrorString())
        return UINT32_MAX;
    }

    // Create & save mesh
    const char* name = scene->mRootNode->mName.C_Str();
    std::string modelName = "Model_";
    std::string meshName = "Mesh_";
    modelName += name;
    meshName += name;
    app->meshes.emplace_back(meshName.c_str());
    Mesh& mesh = app->meshes.back();
    const u32 meshIdx = (u32)app->meshes.size() - 1u;

    // Create & save model, cache mesh idx
    app->models.emplace_back(modelName.c_str());
    Model& model = app->models.back();
    model.meshIdx = meshIdx;
    const u32 modelIdx = (u32)app->models.size() - 1u;

    const std::string directory = GetDirectoryPart(MakeString(filename));

    // Create a list of materials and process each material
    const u32 baseMeshMaterialIndex = (u32)app->materials.size();
    for (unsigned int i = 0; i < scene->mNumMaterials; ++i)
    {
        app->materials.emplace_back();
        Material& material = app->materials.back();
        ProcessAssimpMaterial(app, scene->mMaterials[i], material, directory);
    }

    // Process asset into mesh
    ProcessAssimpNode(scene, scene->mRootNode, &mesh, baseMeshMaterialIndex, model.materialIdx);
    aiReleaseImport(scene);

    // Create VBO & EBO
    u32 vertexBufferSize = 0;
    u32 indexBufferSize = 0;

    // Concatenate all the vertices & indices of the subMeshes into single buffers.
    // Each subMesh then will use their own vertices and indices through vertex offset and indices offset
    for (u32 i = 0; i < mesh.subMeshes.size(); ++i)
    {
        vertexBufferSize += mesh.subMeshes[i].vertices.size() * sizeof(float);
        indexBufferSize  += mesh.subMeshes[i].indices.size()  * sizeof(u32);
    }
    
    mesh.vertexBuffer = CREATE_STATIC_VERTEX_BUFFER(sizeof(vertexBufferSize), nullptr);
    BufferManagement::BindBuffer(mesh.vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, vertexBufferSize, NULL, GL_STATIC_DRAW);

    mesh.indexBuffer = CREATE_STATIC_INDEX_BUFFER(sizeof(indexBufferSize), nullptr);
    BufferManagement::BindBuffer(mesh.indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferSize, NULL, GL_STATIC_DRAW);

    u32 indicesOffset = 0;
    u32 verticesOffset = 0;

    // Batching vertex attributes?? https://learnopengl.com/Advanced-OpenGL/Advanced-Data#:~:text=Batching%20vertex%20attributes
    for (u32 i = 0; i < mesh.subMeshes.size(); ++i)
    {
        const void* verticesData = mesh.subMeshes[i].vertices.data();
        const u32   verticesSize = mesh.subMeshes[i].vertices.size() * sizeof(float);
        glBufferSubData(GL_ARRAY_BUFFER, verticesOffset, verticesSize, verticesData);
        mesh.subMeshes[i].vertexOffset = verticesOffset;
        verticesOffset += verticesSize;

        const void* indicesData = mesh.subMeshes[i].indices.data();
        const u32   indicesSize = mesh.subMeshes[i].indices.size() * sizeof(u32);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, indicesOffset, indicesSize, indicesData);
        mesh.subMeshes[i].indexOffset = indicesOffset;
        indicesOffset += indicesSize;
    }

    BufferManagement::UnBindBuffer(mesh.indexBuffer);
    BufferManagement::UnBindBuffer(mesh.vertexBuffer);

    return modelIdx;
}

void AssimpSupport::ProcessAssimpNode(const aiScene* scene, const aiNode* node, Mesh* myMesh, u32 baseMeshMaterialIndex,
                                      std::vector<u32>& submeshMaterialIndices)
{
    // process all the node's meshes (if any)
    for(unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        ProcessAssimpMesh(scene, mesh, myMesh, baseMeshMaterialIndex, submeshMaterialIndices);
    }

    // then do the same for each of its children
    for(unsigned int i = 0; i < node->mNumChildren; i++)
    {
        ProcessAssimpNode(scene, node->mChildren[i], myMesh, baseMeshMaterialIndex, submeshMaterialIndices);
    }
}

void AssimpSupport::ProcessAssimpMaterial(App* app, const aiMaterial* material, Material& myMaterial, const std::string& directory)
{
    aiString name;
    aiColor3D diffuseColor;
    aiColor3D emissiveColor;
    aiColor3D specularColor;
    ai_real shininess;
    material->Get(AI_MATKEY_NAME, name);
    material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor);
    material->Get(AI_MATKEY_COLOR_EMISSIVE, emissiveColor);
    material->Get(AI_MATKEY_COLOR_SPECULAR, specularColor);
    material->Get(AI_MATKEY_SHININESS, shininess);
    
    myMaterial.name = name.C_Str();
    myMaterial.albedo = vec3(diffuseColor.r, diffuseColor.g, diffuseColor.b);
    myMaterial.emissive = vec3(emissiveColor.r, emissiveColor.g, emissiveColor.b);
    myMaterial.smoothness = shininess / 256.0f;

    aiString aiFilename;
    if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0)
    {
        material->GetTexture(aiTextureType_DIFFUSE, 0, &aiFilename);
        const std::string filename = MakeString(aiFilename.C_Str());
        const std::string filepath = MakePath(directory, filename);
        myMaterial.albedoTextureIdx = TextureSupport::LoadTexture2D(app, filepath.c_str());
    }
    if (material->GetTextureCount(aiTextureType_EMISSIVE) > 0)
    {
        material->GetTexture(aiTextureType_EMISSIVE, 0, &aiFilename);
        const std::string filename = MakeString(aiFilename.C_Str());
        const std::string filepath = MakePath(directory, filename);
        myMaterial.emissiveTextureIdx = TextureSupport::LoadTexture2D(app, filepath.c_str());
    }
    if (material->GetTextureCount(aiTextureType_SPECULAR) > 0)
    {
        material->GetTexture(aiTextureType_SPECULAR, 0, &aiFilename);
        const std::string filename = MakeString(aiFilename.C_Str());
        const std::string filepath = MakePath(directory, filename);
        myMaterial.specularTextureIdx = TextureSupport::LoadTexture2D(app, filepath.c_str());
    }
    if (material->GetTextureCount(aiTextureType_NORMALS) > 0)
    {
        material->GetTexture(aiTextureType_NORMALS, 0, &aiFilename);
        const std::string filename = MakeString(aiFilename.C_Str());
        const std::string filepath = MakePath(directory, filename);
    }
    if (material->GetTextureCount(aiTextureType_HEIGHT) > 0)
    {
        material->GetTexture(aiTextureType_HEIGHT, 0, &aiFilename);
        const std::string filename = MakeString(aiFilename.C_Str());
        const std::string filepath = MakePath(directory, filename);
        myMaterial.normalsTextureIdx = TextureSupport::LoadTexture2D(app, filepath.c_str());

        size_t lastUnderScoreIdx = filename.rfind('_');

        if (lastUnderScoreIdx != std::string::npos) {
            // Replace the part after the last underscore with 'bump'
            std::string bumpFileName = filename.substr(0, lastUnderScoreIdx) + "_bump" + filename.substr(filename.rfind('.')); // Output: xxxx_bump.png
            const std::string bumpFilePath = MakePath(directory, bumpFileName);
            if (std::filesystem::exists(bumpFilePath)) {
                std::cout << "Bump File exists: " << bumpFilePath << std::endl;
                myMaterial.bumpTextureIdx = TextureSupport::LoadTexture2D(app, bumpFilePath.c_str());
            }
        }
    }
    //myMaterial.createNormalFromBump();
}

void AssimpSupport::ProcessAssimpMesh(const aiScene* scene, aiMesh* mesh, Mesh* myMesh, u32 baseMeshMaterialIndex,
                                      std::vector<u32>& submeshMaterialIndices)
{
    std::vector<float> vertices;
    std::vector<u32> indices;

    bool hasTexCoords = false;
    bool hasTangentSpace = false;

    // process vertices
    for(unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        vertices.push_back(mesh->mVertices[i].x);
        vertices.push_back(mesh->mVertices[i].y);
        vertices.push_back(mesh->mVertices[i].z);
        vertices.push_back(mesh->mNormals[i].x);
        vertices.push_back(mesh->mNormals[i].y);
        vertices.push_back(mesh->mNormals[i].z);

        if(mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
        {
            hasTexCoords = true;
            vertices.push_back(mesh->mTextureCoords[0][i].x);
            vertices.push_back(mesh->mTextureCoords[0][i].y);
        }

        if(mesh->mTangents != nullptr && mesh->mBitangents)
        {
            hasTangentSpace = true;
            vertices.push_back(mesh->mTangents[i].x);
            vertices.push_back(mesh->mTangents[i].y);
            vertices.push_back(mesh->mTangents[i].z);

            // For some reason ASSIMP gives me the biTangents flipped.
            // Maybe it's my fault, but when I generate my own geometry
            // in other files (see the generation of standard assets)
            // and all the biTangents have the orientation I expect,
            // everything works ok.
            // I think that (even if the documentation says the opposite)
            // it returns a left-handed tangent space matrix.
            // SOLUTION: I invert the components of the biTangent here.
            vertices.push_back(-mesh->mBitangents[i].x);
            vertices.push_back(-mesh->mBitangents[i].y);
            vertices.push_back(-mesh->mBitangents[i].z);
        }
    }
    // process indices
    for(unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        const aiFace face = mesh->mFaces[i];
        for(unsigned int j = 0; j < face.mNumIndices; j++)
        {
            indices.push_back(face.mIndices[j]);
        }
    }

    // store the proper (previously processed) material for this mesh
    submeshMaterialIndices.push_back(baseMeshMaterialIndex + mesh->mMaterialIndex); // Base + offset

    // create the vertex format
    VertexBufferLayout vertexBufferLayout = {};
    vertexBufferLayout.attributes.push_back( VertexBufferAttribute{ static_cast<int>(VERTEX_ATTRIBUTE_LOCATION::ATTR_LOCATION_POSITION), 3, 0 } );
    vertexBufferLayout.attributes.push_back( VertexBufferAttribute{ static_cast<int>(VERTEX_ATTRIBUTE_LOCATION::ATTR_LOCATION_NORMAL), 3, 3*sizeof(float) } );
    vertexBufferLayout.stride = 6 * sizeof(float);
    if (hasTexCoords)
    {
        vertexBufferLayout.attributes.push_back( VertexBufferAttribute{ static_cast<int>(VERTEX_ATTRIBUTE_LOCATION::ATTR_LOCATION_TEXTCOORD), 2, vertexBufferLayout.stride } );
        vertexBufferLayout.stride += 2 * sizeof(float);
    }
    if (hasTangentSpace)
    {
        vertexBufferLayout.attributes.push_back( VertexBufferAttribute{ static_cast<int>(VERTEX_ATTRIBUTE_LOCATION::ATTR_LOCATION_TANGENT), 3, vertexBufferLayout.stride } );
        vertexBufferLayout.stride += 3 * sizeof(float);

        vertexBufferLayout.attributes.push_back( VertexBufferAttribute{ static_cast<int>(VERTEX_ATTRIBUTE_LOCATION::ATTR_LOCATION_BITANGENT), 3, vertexBufferLayout.stride } );
        vertexBufferLayout.stride += 3 * sizeof(float);
    }

    // Add the subMesh into the mesh
    SubMesh subMesh = {mesh->mName.C_Str()};
    subMesh.vertexBufferLayout = vertexBufferLayout;
    subMesh.vertices.swap(vertices);
    subMesh.indices.swap(indices);
    myMesh->subMeshes.push_back( subMesh );
}
