#pragma once

#include "game.h"

namespace Core
{
namespace Components
{
class CORE_API Camera;
}

namespace Physics
{
class CORE_API State;
}
} // namespace Core

namespace Game
{
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
extern GAME_API flecs::entity Create(flecs::world& world,
                              Core::Physics::State& physics,
                              Core::Components::Camera** camera);

// Create a projectile for the player
extern GAME_API flecs::entity CreateProjectile(flecs::entity player,
                                        Core::Physics::State& physics,
                                        float lifespan, float speed);

// Handle input
extern GAME_API void HandleInput(flecs::iter& iter);

// Get the cursor's position in the world
extern GAME_API PxVec3 GetCursorPosition(flecs::entity player, float distance = 5);

// Draw the cursor
extern GAME_API void DrawCursor(flecs::iter& iter);
} // namespace Player
} // namespace Game