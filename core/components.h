#pragma once

#include "stdafx.h"

namespace Core
{
namespace Tags
{
// Is a projectile
struct Projectile
{
};
} // namespace Tags

namespace Components
{
// Register components
extern CORE_API void Register(flecs::world& world);

// Timeout in seconds
struct Timeout
{
    float seconds;
};

// Health
struct Health
{
    float value;
    float max;
};

// Movement speeds
struct MovementSpeed
{
    float walk;
    float crouch;
    float run;
};
} // namespace Components
} // namespace Core