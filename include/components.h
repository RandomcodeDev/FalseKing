#pragma once

#include "game.h"
#include "input.h"

namespace Components
{
// Set up components
extern void Register(flecs::world& world);

// Is a player
struct Player
{
};

// Is the player on the local instance
struct LocalPlayer
{
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
    float run;
};
} // namespace Components
