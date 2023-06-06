#pragma once

#include <chrono>
#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

#include "box2d/box2d.h"

#include "entt/entt.hpp"

#include "glm/vec2.hpp"
#include "glm/vec4.hpp"

#include "qoi.h"

#include "spdlog/spdlog.h"
#include "spdlog/fmt/fmt.h"
#include "spdlog/fmt/chrono.h"

namespace chrono = std::chrono;
namespace fs = std::filesystem;
using precise_clock = chrono::high_resolution_clock;

// Constants

constexpr const char* GAME_NAME = "Game"; // TODO: come up with name

// Physics time step
constexpr float PHYSICS_STEP = 1.0 / 60.0;

// Physics gravity
constexpr float PHYSICS_GRAVITY = 9.807;

// Physics iteration counts (these are the ones recommended in the docs)
constexpr int PHYSICS_VELOCITY_ITERATIONS = 8;
constexpr int PHYSICS_POSITION_ITERATIONS = 3;

// Exit the program
[[noreturn]]
extern void Quit(const std::string& message, int32_t exitCode);
