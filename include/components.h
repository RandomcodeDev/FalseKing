#pragma once

#include "game.h"
#include "input.h"

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
} // namespace Components
