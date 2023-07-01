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
