#include "systems.h"
#include "backend.h"
#include "discord.h"
#include "player.h"
#include "sprites.h"

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
        .with(EcsTraverseAll)
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
    ImGui::NewFrame();
}

void Systems::EndRender(flecs::iter& iter)
{
    ImGui::Render();
    g_backend->EndRender();
}

PxVec3 DEBUG_TEXT_COLOR = PxVec3(0, 1, 0);
void Systems::DebugInfo(flecs::iter& iter)
{
    Context* context = iter.ctx<Context>();
    float fps = 1.0f / iter.delta_time();
    ImGui::Begin("Debug Information", nullptr, ImGuiWindowFlags_DummyWindow);
    ImGui::SetWindowPos(ImVec2(0, 0));
    ImGui::SetWindowSize(ImVec2((float)g_backend->GetWindowInformation().width,
                                ImGui::GetFontSize() * 18));
    IMGUI_TEXT("{} v{}.{}.{} commit {}", GAME_NAME, GAME_MAJOR_VERSION,
               GAME_MINOR_VERSION, GAME_PATCH_VERSION, GAME_COMMIT);
    IMGUI_TEXT("FPS:{:0.3}", fps);
    IMGUI_TEXT("Frame delta: {:0.3} ms", iter.delta_time() * 1000.0f);
    IMGUI_TEXT("Frames renderered: {}", g_backend->GetFrameCount());
    IMGUI_TEXT("Total runtime: {:%T}",
                            precise_clock::now() - context->startTime);
    IMGUI_TEXT("Entity count: {}", iter.count());
    IMGUI_TEXT("System: {}", g_backend->DescribeSystem());
    IMGUI_TEXT("Backend: {}", g_backend->DescribeBackend());
    IMGUI_TEXT("Discord: {}, {}",
                    Discord::Available() ? "available" : "not available",
                    Discord::Connected() ? "connected" : "not connected");
    ImGui::Text("Input state:");
    ImGui::TextWrapped("%s", context->input->DescribeState().c_str());
    ImGui::End();
}

void Systems::KillTimedout(const flecs::entity& entity,
                           Components::Timeout& timeout)
{
    timeout.seconds -= entity.world().delta_time();
    if (timeout.seconds <= 0.0f)
    {
        entity.destruct();
    }
}
