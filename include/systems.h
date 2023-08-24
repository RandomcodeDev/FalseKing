#pragma once

#include "stdafx.h"

namespace Components
{
class Camera;
struct Timeout;
} // namespace Components

namespace Input
{
class State;
}

namespace Physics
{
class State;
}

namespace Systems
{

// Debug cycle cooldown (so it actually works)
constexpr chrono::milliseconds DEBUG_CYCLE_COOLDOWN = 200ms;

// Debug view mode
enum class DebugMode : uint32_t
{
    None = 0b0, // Nothing
    TextOverlay = 0b1, // Text overlay with FPS and other stuff
    Physics = 0b10, // PhysX visualization
    Count, // Number of modes
    All = 0xFFFFFFFF, // Everything
};

// Information systems can use
struct Context
{
    precise_clock::time_point startTime;
    Input::State* input;
    Physics::State* physics;
    Components::Camera* mainCamera;
    DebugMode debugMode;
};

// Register all the systems
extern void Register(flecs::world& world, Context* context);

// Begin rendering
extern void BeginRender(flecs::iter& iter);

// End rendering
extern void EndRender(flecs::iter& iter);

// Show debug info
extern void ShowDebugOverlay(flecs::iter& iter);

// Kill off timed out entities
extern void KillTimedout(const flecs::entity& entity,
                         Components::Timeout& timeout);
} // namespace Systems
