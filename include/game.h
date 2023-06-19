#pragma once

#include <algorithm>
#include <cerrno>
#include <chrono>
#include <cinttypes>
#include <codecvt>
#include <cstdlib>
#include <filesystem>
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
#include "spdlog/fmt/chrono.h"
#include "spdlog/fmt/fmt.h"
#include "spdlog/spdlog.h"

#define TOML_FLOAT_CHARCONV 0
#include "toml++/toml.h"

#include "zstd.h"

namespace chrono = std::chrono;
namespace fs = std::filesystem;
using precise_clock = chrono::high_resolution_clock;

// Constants

constexpr const char* GAME_NAME = "False King";

// Game screen size
constexpr uint32_t GAME_WIDTH = 256;
constexpr uint32_t GAME_HEIGHT = 168;

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
