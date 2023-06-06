#pragma once

#include "game.h"

// Transformation
struct Transform
{
    glm::vec2 position;
    glm::vec2 scale;
    float rotation;

    Transform()
        : position(), scale(), rotation()
    {}

    Transform(const glm::vec2& position, const glm::vec2& scale, float rotation)
        : position(position), scale(scale), rotation(rotation)
    {}
};
