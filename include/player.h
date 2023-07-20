#pragma once

#include "components.h"
#include "game.h"
#include "physics.h"
#include "systems.h"

// Player helper functions
namespace Player
{
// Is a player
struct Player
{
};

// Is the player on the local instance
struct LocalPlayer
{
};

// The position of the player's cursor
struct Cursor
{
    float x;
    float y;
};

// The cooldown on the player's melee attack
struct MeleeCooldown
{
    float value;
};

// Create the player entity
flecs::entity Create(flecs::world& world, Physics::State& physics);

// Create a projectile for the player
flecs::entity CreateProjectile(flecs::entity player, Physics::State& physics);

// Handle input
void Input(flecs::iter& iter);

// Get the cursor's position in the world
PxVec3 GetCursorPosition(flecs::entity player, float distance = 5);

// Draw the cursor
void DrawCursor(flecs::iter& iter);
} // namespace Player