#pragma once

#include "components.h"
#include "game.h"
#include "input.h"
#include "physics.h"
#include "sprite.h"

namespace Systems
{
// Information systems can use
struct Context
{
    Input::State* input;
    Physics::State* physics;
    precise_clock::time_point startTime;
};

// Register all the systems
extern void Register(flecs::world& world, Context* context);

// Handle input
extern void PlayerInput(flecs::iter& iter);

// Begin rendering
extern void BeginRender(flecs::iter& iter);

// End rendering
extern void EndRender(flecs::iter& iter);

// Show debug info
extern void DebugInfo(flecs::iter& iter);
} // namespace Systems
