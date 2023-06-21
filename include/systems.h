#pragma once

#include "components.h"
#include "game.h"
#include "input.h"
#include "physics.h"
#include "sprite.h"

namespace Systems
{
// Register all the systems
extern void Register(flecs::world& world, InputState* input);

// Handle input
extern void PlayerInput(flecs::iter& iter);
} // namespace Systems
