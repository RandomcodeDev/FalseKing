#pragma once

#include "game.h"
#include "input.h"
#include "physics.h"

namespace Tags
{
// Is a player
struct Player
{
};

// Is the player on the local instance
struct LocalPlayer
{
};

// Is a projectile
struct Projectile
{
};
} // namespace Tags

namespace Components
{
// Register components
void Register(flecs::world& world);

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

// Element
struct Element
{
    enum class Enum
    {
        None,
        Fire,
        Water,
        Air,
        Earth,
        Ultimate
    } value;
};
} // namespace Components
