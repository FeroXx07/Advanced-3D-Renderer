#ifndef LIGHT_H
#define LIGHT_H
#include "platform.h"

enum class LightType
{
    DIRECTIONAL = 0,
    POINT = 1
};

struct Attenuation
{
    float constant = 1.0f;
    float linear = 0.7f;
    float quadratic = 1.8f;
    float radius;

    inline void CalculateRadius(const glm::vec3& lightColor)
    {
        const float lightMax  = std::fmaxf(std::fmaxf(lightColor.r, lightColor.g), lightColor.b);
        radius = (-linear +  std::sqrtf(linear * linear - 4.0f * quadratic * (constant - (256.0f / 5.0f) * lightMax))) / (2.0f * quadratic);  
    }
};

struct Light : public Entity
{
    LightType type;
    Attenuation attenuation;
};

static const char* LightTypeNames[] = 
  {
    "DIRECTIONAL",
    "POINT",
    };
#endif // LIGHT_H
