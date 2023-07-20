#pragma once

#include "components.h"
#include "game.h"
#include "physics.h"
#include "systems.h"

// Player helper functions
namespace Player
{
// Create the player entity
flecs::entity Create(flecs::world& world, Physics::State& physics);

// Create a projectile for the player
flecs::entity CreateProjectile(flecs::entity player, Physics::State& physics);

// Handle input
void Input(flecs::iter& iter);
}
