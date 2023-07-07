#include "systems.h"
#include "backend.h"
#include "discord.h"
#include "text.h"

void Systems::Register(flecs::world& world, Context* context)
{
    // physics.h
    world.system("PhysicsUpdate")
        .ctx(context)
        .kind(flecs::OnUpdate)
        .multi_threaded()
        .interval(PhysicsState::TIME_STEP)
        .iter(PhysicsUpdate);

    // systems.h
    world.system<const Tags::LocalPlayer>("PlayerInput")
        .ctx(context)
        .kind(flecs::OnUpdate)
        .interval(PhysicsState::TIME_STEP)
        .iter(PlayerInput);
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

    // sprite.h
    world.system<PhysicsController, const Sprite>("DrawControlled")
        .kind(flecs::OnUpdate)
        .iter(DrawControlled);
}

void Systems::PlayerInput(flecs::iter& iter)
{
    Context* context = iter.ctx<Context>();
    InputState* input = context->input;

    flecs::entity player = iter.entity(0);
    auto controller = player.get_mut<PhysicsController>();
    auto movementSpeed = player.get<Components::MovementSpeed>();

    float x = input->GetLeftStickDirection().x *
              (input->GetLeftStickPressed() ? movementSpeed->run
                                            : movementSpeed->walk) *
              (input->GetRightStickPressed() ? movementSpeed->crouch
                                             : movementSpeed->walk);
    float z = input->GetLeftStickDirection().y *
              (input->GetLeftStickPressed() ? movementSpeed->run
                                            : movementSpeed->walk) *
              (input->GetRightStickPressed() ? movementSpeed->crouch
                                             : movementSpeed->walk);

    controller->GetController().move(PxVec3(x, 0, z), 0.0001f,
                                     iter.delta_system_time() * 1000.0f,
                                     PxControllerFilters());
}

void Systems::BeginRender(flecs::iter& iter)
{
    g_backend->BeginRender();
}

void Systems::EndRender(flecs::iter& iter)
{
    g_backend->EndRender();
}

void Systems::DebugInfo(flecs::iter& iter)
{
    Context* context = iter.ctx<Context>();
    float fps = 1.0f / iter.delta_time();
    Text::DrawString(
        fmt::format(
            "FPS: {:0.3}\nFrame delta: {} ms\nFrames rendered: {}\n{} "
            "v{}.{}.{} commit {}\nTotal "
            "runtime: {:%T}\nSystem: {}\nBackend: {}\nDiscord: {}, {}\n\n\n{}",
            fps, 1000.0f * iter.delta_time(), g_backend->GetFrameCount(),
            GAME_NAME, GAME_MAJOR_VERSION, GAME_MINOR_VERSION,
            GAME_PATCH_VERSION, GAME_COMMIT,
            precise_clock::now() - context->startTime,
            g_backend->DescribeSystem(), g_backend->DescribeBackend(),
            Discord::Available() ? "available" : "not available",
            Discord::Connected() ? "connected" : "not connected",
            context->input->DescribeState()),
        glm::uvec2(0, 0), DEBUG_TEXT_SCALE, DEBUG_TEXT_COLOR);
}
