#pragma once
#include <vector>
#include "platform.h"

struct ScreenSpaceAmbientOcclusion
{
    std::vector<glm::vec3> kernel;
    std::vector<glm::vec3> noise;
    const int maxSamples = 64;
    int kernelSize = 64;
    float radius = 0.5f;
    float bias = 0.025f;

    // tile noise texture over screen based on screen dimensions divided by noise size
    glm::vec2 noiseScale;

    u32 paramsOffset;
    u32 paramsSize;
};
