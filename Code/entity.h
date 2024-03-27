#ifndef ENTITY_H
#define ENTITY_H
#include "platform.h"

struct Entity
{
    std::string name;

    glm::vec4 color;
    glm::mat4 worldMatrix;
    glm::mat4 worldViewProjectionMat;
    u32 modelIndex;
    u32 programIndex;
    u32 localParamsOffset;
    u32 localParamsSize;
};
#endif // ENTITY_H
