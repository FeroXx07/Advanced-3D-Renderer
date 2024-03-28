#ifndef ENTITY_H
#define ENTITY_H
#include "platform.h"

enum class EntityType
{
    BASE = 0,
    LIGHT = 1
};

struct Entity
{
    std::string name;
    EntityType eType = EntityType::BASE;
    
    glm::vec4 color;
    
    glm::mat4 worldMatrix;
    glm::mat4 worldViewProjectionMat;
    
    glm::vec3 position;
    glm::vec3 orientationEuler;
    glm::quat orientationQuat;
    glm::vec3 scale;
    
    u32 modelIndex;
    u32 programIndex;
    u32 localParamsOffset;
    u32 localParamsSize;
};
#endif // ENTITY_H
