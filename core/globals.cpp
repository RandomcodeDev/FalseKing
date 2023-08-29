#include "game/game.h"

#include "backend.h"
#include "stdafx.h"

CORE_API Core::Backend* Core::g_backend;

CORE_API const char* Core::GAME_COMMIT = {
#include "commit.txt"
};

[[noreturn]] CORE_API void Core::Quit(int32_t exitCode, const std::string& message)
{
    g_backend->Quit(exitCode, message);
}