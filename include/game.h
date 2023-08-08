#pragma once

#include <algorithm>
#include <cerrno>
#include <chrono>
#include <codecvt>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <locale>
#include <map>
#include <sstream>
#include <string>

#ifndef TOOL
#include "discord/discord.h"

#include "flecs.h"

#define PX_STATIC 1
#define PX_PHYSX_STATIC_LIB 1
#include "PxPhysicsAPI.h"
#include "characterkinematic/PxController.h"
#endif

#include "imgui.h"
#include "imgui_internal.h"

#include "metrohash.h"
using namespace physx;

#define QOI_NO_STDIO
#include "qoi.h"

#define FMT_HEADER_ONLY
#define SPDLOG_HEADER_ONLY
#ifdef _DEBUG
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
#endif
#define FMT_NO_EXCEPTIONS
#define SPDLOG_NO_EXCEPTIONS
#include "spdlog/fmt/chrono.h"
#include "spdlog/fmt/fmt.h"
#include "spdlog/sinks/base_sink.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/sink.h"
#include "spdlog/spdlog.h"
#ifdef _WIN32
#include "spdlog/sinks/msvc_sink.h"
#endif

#include "toml.h"

#include "zstd.h"

// Windows redefines some things, that could cause issues
#undef LoadLibrary

namespace chrono = std::chrono;
using precise_clock = chrono::high_resolution_clock;

// Constants

constexpr const char* GAME_NAME = "False King";
constexpr uint8_t GAME_MAJOR_VERSION = 0;
constexpr uint8_t GAME_MINOR_VERSION = 0;
constexpr uint8_t GAME_PATCH_VERSION = 0;
constexpr ImGuiWindowFlags ImGuiWindowFlags_DummyWindow =
    ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
    ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
    ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoInputs;

extern const char* GAME_COMMIT;

// Game screen size
constexpr uint32_t GAME_WIDTH = 256;
constexpr uint32_t GAME_HEIGHT = 168;

// Debug font settings
extern PxVec3 DEBUG_TEXT_COLOR;

// Get the size of an array
template <class T, size_t N> constexpr size_t ARRAY_SIZE(T (&)[N])
{
    return N;
}

// Get the sign of a number
// https://stackoverflow.com/questions/1903954/is-there-a-standard-sign-function-signum-sgn-in-c-c
template <typename T> T SIGN(T val)
{
    return (T(0) < val) - (val < T(0));
}

// Exit the program
[[noreturn]] extern void Quit(const std::string& message, int32_t exitCode = 1);
#define QUIT(...) Quit(fmt::format(__VA_ARGS__))
#define QUIT_CODE(exitCode, ...) Quit(fmt::format(__VA_ARGS__), exitCode)

// ImGui text
#define IMGUI_TEXT_(Type, ...) ImGui::Text##Type("%s", fmt::format(__VA_ARGS__).c_str())
#define IMGUI_TEXT(...) IMGUI_TEXT_(, __VA_ARGS__)
#define IMGUI_TEXT_WRAPPED(...) IMGUI_TEXT_(Wrapped, __VA_ARGS__)