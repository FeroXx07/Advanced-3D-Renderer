#ifndef LIGHT_H
#define LIGHT_H
#include "platform.h"

enum class LightType
{
    DIRECTIONAL = 0,
    POINT = 1
};

struct Light
{
    LightType type;
    glm::vec4 color;
    glm::vec3 direction;
    glm::vec3 position;
};

static const char* LightTypeNames[] = 
  {
    "DIRECTIONAL",
    "POINT",
    };
#endif // LIGHT_H
