#ifndef LIGHT_H
#define LIGHT_H
#include "platform.h"

enum class LightType
{
    DIRECTIONAL = 0,
    POINT = 1
};

struct Light : public Entity
{
    LightType type;
};

static const char* LightTypeNames[] = 
  {
    "DIRECTIONAL",
    "POINT",
    };
#endif // LIGHT_H
