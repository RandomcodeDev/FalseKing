#pragma once

#include <algorithm>
#include <cerrno>
#include <chrono>
#include <cinttypes>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include "box2d/box2d.h"

#ifndef __WINRT__
#include "entt/entt.hpp"
#endif

#include "glm/vec2.hpp"
#include "glm/vec4.hpp"

#include "metrohash.h"

#define QOI_NO_STDIO
#include "qoi.h"

#define FMT_HEADER_ONLY
#define SPDLOG_HEADER_ONLY
#include "spdlog/fmt/chrono.h"
#include "spdlog/fmt/fmt.h"
#include "spdlog/spdlog.h"

#include "zstd.h"

namespace chrono = std::chrono;
namespace fs = std::filesystem;
using precise_clock = chrono::high_resolution_clock;

// Constants

constexpr const char* GAME_NAME = "Game"; // TODO: come up with name

// Game screen resolution
constexpr uint32_t GAME_WIDTH = 256;
constexpr uint32_t GAME_HEIGHT = 168;

// Physics time step
constexpr float PHYSICS_STEP = 1.0f / 60.0f;

// Physics gravity
constexpr float PHYSICS_GRAVITY = 9.807f;

// Physics iteration counts (these are the ones recommended in the docs)
constexpr uint32_t PHYSICS_VELOCITY_ITERATIONS = 8;
constexpr uint32_t PHYSICS_POSITION_ITERATIONS = 3;

// Exit the program
[[noreturn]] extern void Quit(const std::string& message, int32_t exitCode = 1);
