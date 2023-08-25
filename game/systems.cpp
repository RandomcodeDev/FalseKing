#include "systems.h"
#include "backend.h"
#include "camera.h"
#include "components.h"
#include "discord.h"
#include "input.h"
#include "physics.h"
#include "player.h"
#include "sprites.h"

void Systems::Register(flecs::world& world, Context* context)
{
    // player.h
    world.system<const Player::LocalPlayer>("PlayerInput")
        .ctx(context)
        .kind(flecs::OnUpdate)
        .interval(Physics::TIME_STEP)
        .iter(Player::HandleInput);
    world.system<const Player::LocalPlayer>("DrawCursor")
        .kind(flecs::OnUpdate)
        .iter(Player::DrawCursor);
}