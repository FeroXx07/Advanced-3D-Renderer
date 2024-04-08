#ifndef MESH_EXAMPLE_H
#define MESH_EXAMPLE_H
#include "app.h"
#include "mesh.h"
#include "vertex.h"

inline u32 CreateSampleMesh(App* app)
{
     /* TODO: Initialize your resources here! */
    // - vertex buffers
    // - element/index buffers
    // - VAOs
    constexpr float planeHalfWidth = 10.0f/2.0f;
    constexpr float planeHalfHeight = 10.0f/2.0f;

    constexpr VertexV3V2 vertices[] = {
        {glm::vec3(0.0 - planeHalfWidth, 0.0, 0.0 - planeHalfHeight), glm::vec3(0.0, 1.0, 0.0),glm::vec2(0.0, 0.0)}, // bottom-left vertex
        {glm::vec3(0.0 + planeHalfWidth, 0.0, 0.0 - planeHalfHeight), glm::vec3(0.0, 1.0, 0.0),glm::vec2(1.0, 0.0)}, // bottom-right vertex
        {glm::vec3(0.0 + planeHalfWidth, 0.0, 0.0 + planeHalfHeight), glm::vec3(0.0, 1.0, 0.0),glm::vec2(1.0, 1.0)}, // top-right vertex
        {glm::vec3(0.0 - planeHalfWidth, 0.0, 0.0 + planeHalfHeight), glm::vec3(0.0, 1.0, 0.0), glm::vec2(0.0, 1.0)}, // top-left vertex
    };

    const u32 indices[] = {
        0, 1, 2,
        0, 2, 3
    };
    Model model = {"MyPlaneModel"};
    Mesh mesh = {"MyPlaneMesh"};
    
    app->materials.emplace_back();
    Material& material = app->materials.back();
    material.name = "MyPlane_Dice_Mat";
    material.albedo = glm::vec3(255);
    material.albedoTextureIdx = app->colorTextureIdx;
    model.materialIdx.push_back(app->materials.size() - 1);
    // Geometry (vertex buffer object & element buffer object, gpu side)
    mesh.vertexBuffer = CREATE_STATIC_VERTEX_BUFFER(sizeof(vertices), (void*)vertices);
    mesh.indexBuffer = CREATE_STATIC_INDEX_BUFFER(sizeof(indices), (void*)indices);
    /*glGenBuffers(1, &mesh.vertexBuffer.handle);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &mesh.indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);*/

    // Attribute state, VAO is like an object to store client side, bind vbo with ebo, with shaders, etc...
    /*glGenVertexArrays(1, &app->vao);
    glBindVertexArray(app->vao);

    glBindBuffer(GL_ARRAY_BUFFER, app->embeddedVertices);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexV3V2), (void*)0); //layout (location = 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexV3V2), (void*)sizeof(glm::vec3)); //layout (location = 1)
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, app->embeddedElements);

    glBindVertexArray(0);
    */
    VertexBufferLayout vertexBufferLayout = {};
    vertexBufferLayout.attributes.push_back(VertexBufferAttribute{0, 3, 0}); // 3D positions
    vertexBufferLayout.attributes.push_back(VertexBufferAttribute{1, 3, sizeof(glm::vec3)}); // normals
    vertexBufferLayout.attributes.push_back(VertexBufferAttribute{2, 2, sizeof(glm::vec3) * 2}); // 2D tex coords
    vertexBufferLayout.stride = sizeof(VertexV3V2);
    
    SubMesh subMesh = {"MyPlaneSubMesh"};
    subMesh.vertexBufferLayout = vertexBufferLayout;

    constexpr u32 verticesSize = std::size(vertices);
    for (u32 i = 0; i < verticesSize; ++i)
    {
        std::vector<float> vec = vertices[i].GetVector();
        subMesh.vertices.insert(subMesh.vertices.end(), vec.begin(), vec.end());
    }
    
    subMesh.indices = std::vector<u32>(indices, indices + std::size(indices));
    mesh.subMeshes.push_back(subMesh);

    app->meshes.push_back(mesh);
    model.meshIdx = (u32)app->meshes.size() - 1;
    app->models.push_back(model);
    
    u32 modelIdx = app->models.size() - 1;
    return modelIdx;
}
#endif // MESH_EXAMPLE_H
