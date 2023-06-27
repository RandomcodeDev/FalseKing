#pragma once

#include <algorithm>
#include <cerrno>
#include <chrono>
#include <cinttypes>
#include <codecvt>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <locale>
#include <string>
#include <thread>

#ifndef TOOL
#include "flecs.h"

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#endif

#include "metrohash.h"

#include "PxPhysicsAPI.h"
#include "characterkinematic/PxController.h"
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
#include "spdlog/spdlog.h"
#include "spdlog/sinks/sink.h"
#include "spdlog/sinks/base_sink.h"
#ifdef _WIN32
#include "spdlog/sinks/msvc_sink.h"
#endif

#define TOML_FLOAT_CHARCONV 0
#define TOML_EXCEPTIONS 0
#include "toml++/toml.h"

#include "zstd.h"

namespace chrono = std::chrono;
using precise_clock = chrono::high_resolution_clock;

// Constants

constexpr const char* GAME_NAME = "False King";

// Game screen size
constexpr uint32_t GAME_WIDTH = 256;
constexpr uint32_t GAME_HEIGHT = 168;

// Frame smoothing
constexpr float FRAME_SMOOTHING = 0.9f;

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
