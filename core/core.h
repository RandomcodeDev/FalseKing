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

#include "imgui.h"
#include "imgui_internal.h"
#endif

#include "CLI11.hpp"

#include "foundation/Px.h"
#include "foundation/PxMath.h"
#include "foundation/PxVecMath.h"
using namespace physx;

#include "metrohash.h"

#define QOI_NO_STDIO
#include "qoi.h"

#define FMT_HEADER_ONLY
#define SPDLOG_HEADER_ONLY
#ifdef _DEBUG
// This only enables the macros, the level of the default level actuall controls
// it.
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
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
using namespace std::chrono_literals;
using precise_clock = chrono::high_resolution_clock;

#ifdef _MSC_VER
#define ATTRIBUTE(x) __declspec(x)
#else
#define ATTRIBUTE(x) __attribute__((x))
#endif

#ifdef CORE
#ifdef _WIN32
#define CORE_API ATTRIBUTE(dllexport)
#else
#define CORE_API ATTRIBUTE(visibility("default"))
#endif
#else
#ifdef _WIN32
#define CORE_API ATTRIBUTE(dllimport)
#else
#define CORE_API
#endif
#endif

namespace Core
{

// Constants

constexpr const char* GAME_NAME = "False King";
constexpr uint8_t GAME_MAJOR_VERSION = 0;
constexpr uint8_t GAME_MINOR_VERSION = 0;
constexpr uint8_t GAME_PATCH_VERSION = 0;

#ifndef TOOL
constexpr ImGuiWindowFlags ImGuiWindowFlags_DummyWindow =
    ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
    ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
    ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoInputs;
#endif

extern CORE_API const char* GAME_COMMIT;

// Game screen size
constexpr uint32_t GAME_WIDTH = 256;
constexpr uint32_t GAME_HEIGHT = 168;

// Debug font settings
extern CORE_API PxVec3 DEBUG_TEXT_COLOR;

// Get the size of an array
template <class T, size_t N> constexpr size_t ArraySize(T (&)[N])
{
    return N;
}

// Get the sign of a number
// https://stackoverflow.com/questions/1903954/is-there-a-standard-sign-function-signum-sgn-in-c-c
template <typename T> inline T Sign(T val)
{
    return (T(0) < val) - (val < T(0));
}

// Unpack a number from an int into a vec4
inline PxVec4 UnpackColor(uint32_t color)
{
    return PxVec4((float)(color >> 24 & 0xFF), (float)(color >> 16 & 0xFF),
                  (float)(color >> 8 & 0xFF), (float)(color >> 0 & 0xFF));
}

// Exit the program
[[noreturn]] CORE_API void Quit(int32_t exitCode, const std::string& message);

template <typename... Args>
[[noreturn]] void Quit(int32_t exitCode, const std::string& message,
                       Args... args)
{
    Quit(exitCode, fmt::format(message, args...));
}
template <typename... Args>
[[noreturn]] void Quit(const std::string& message, Args... args)
{
    Quit(1, message, args...);
}

// ImGui text
#define IMGUI_TEXT_(Type, ...)                                                 \
    ImGui::Text##Type("%s", fmt::format(__VA_ARGS__).c_str())
#define IMGUI_TEXT(...) IMGUI_TEXT_(, __VA_ARGS__)
#define IMGUI_TEXT_WRAPPED(...) IMGUI_TEXT_(Wrapped, __VA_ARGS__)

} // namespace Core
