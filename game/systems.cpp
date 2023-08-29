#include "core/backend.h"
#include "core/camera.h"
#include "core/discord.h"
#include "core/input.h"
#include "core/physics.h"

#include "components.h"
#include "player.h"
#include "sprites.h"
#include "systems.h"

GAME_API void Game::Systems::Register(flecs::world& world,
                             Core::Systems::Context* context)
{
    // player.h
    world.system<const Player::LocalPlayer>("PlayerInput")
        .ctx(context)
        .kind(flecs::OnUpdate)
        .interval(Core::Physics::TIME_STEP)
        .iter(Player::HandleInput);
    world.system<const Player::LocalPlayer>("DrawCursor")
        .kind(flecs::OnUpdate)
        .iter(Player::DrawCursor);
}