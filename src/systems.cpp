#include "systems.h"
#include "backend.h"
#include "discord.h"
#include "player.h"
#include "sprites.h"
#include "text.h"

void Systems::Register(flecs::world& world, Context* context)
{
    // physics.h
    world.system("PhysicsUpdate")
        .ctx(context)
        .kind(flecs::OnUpdate)
        .multi_threaded()
        .interval(Physics::TIME_STEP)
        .iter(Physics::Update);

    // player.h
    world.system<const Player::LocalPlayer>("PlayerInput")
        .ctx(context)
        .kind(flecs::OnUpdate)
        .interval(Physics::TIME_STEP)
        .iter(Player::Input);
    world.system<const Player::LocalPlayer>("DrawCursor")
        .kind(flecs::OnUpdate)
        .iter(Player::DrawCursor);

    // systems.h
    world.system("BeginRender")
        .ctx(context)
        .kind(flecs::PreFrame)
        .iter(BeginRender);
    world.system("EndRender")
        .ctx(context)
        .kind(flecs::PostFrame)
        .iter(EndRender);
    world.system("DebugInfo")
        .ctx(context)
        .kind(flecs::PostUpdate)
        .iter(DebugInfo);
    world.system<Components::Timeout>("KillTimedout")
        .kind(flecs::PostFrame)
        .each(KillTimedout);

    // sprite.h
    world.system<Physics::Base, const Sprite>("DrawPhysical")
        .kind(flecs::OnUpdate)
        .each(DrawPhysical);
    world.system<Physics::Body, const Sprite>("DrawBody")
        .kind(flecs::OnUpdate)
        .each(DrawBody);
    world.system<Physics::Controller, const Sprite>("DrawController")
        .kind(flecs::OnUpdate)
        .each(DrawController);
}

void Systems::BeginRender(flecs::iter& iter)
{
    g_backend->BeginRender();
}

void Systems::EndRender(flecs::iter& iter)
{
    g_backend->EndRender();
}

PxVec3 DEBUG_TEXT_COLOR = PxVec3(0, 1, 0);
void Systems::DebugInfo(flecs::iter& iter)
{
    Context* context = iter.ctx<Context>();
    float fps = 1.0f / iter.delta_time();
    Text::DrawString(
        fmt::format("FPS: {:0.3}\nFrame delta: {} ms\nFrames rendered: {}\n{} "
                    "v{}.{}.{} commit {}\nTotal "
                    "runtime: {:%T}\nEntity count: {}\nSystem: {}\nBackend: {}\nDiscord: {}, "
                    "{}\n\nInput:\n{}",
                    fps, 1000.0f * iter.delta_time(),
                    g_backend->GetFrameCount(), GAME_NAME, GAME_MAJOR_VERSION,
                    GAME_MINOR_VERSION, GAME_PATCH_VERSION, GAME_COMMIT,
                    precise_clock::now() - context->startTime, iter.count(),
                    g_backend->DescribeSystem(), g_backend->DescribeBackend(),
                    Discord::Available() ? "available" : "not available",
                    Discord::Connected() ? "connected" : "not connected",
                    context->input->DescribeState()),
        PxVec2(0, 0), DEBUG_TEXT_SCALE, DEBUG_TEXT_COLOR);
}

void Systems::KillTimedout(flecs::entity& entity, Components::Timeout& timeout)
{
    timeout.seconds -= entity.world().delta_time();
    if (timeout.seconds <= 0.0f)
    {
        entity.destruct();
    }
}
