#pragma once

#include "stdafx.h"

namespace Components
{
class Camera;
}

namespace Physics
{
class State;
}

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

constexpr float BASE_MELEE_COOLDOWN = 0.07f;

// The cooldown on the player's melee attack
struct MeleeCooldown
{
    float value;
};

// Create the player entity
flecs::entity Create(flecs::world& world, Physics::State& physics, Components::Camera** camera);

// Create a projectile for the player
flecs::entity CreateProjectile(flecs::entity player, Physics::State& physics,
                               float lifespan, float speed);

// Handle input
void HandleInput(flecs::iter& iter);

// Get the cursor's position in the world
PxVec3 GetCursorPosition(flecs::entity player, float distance = 5);

// Draw the cursor
void DrawCursor(flecs::iter& iter);
} // namespace Player
