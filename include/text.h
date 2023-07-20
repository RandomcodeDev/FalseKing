#pragma once

#include "backend.h"
#include "fs.h"
#include "game.h"
#include "image.h"

namespace Text
{
// Character size
constexpr uint32_t CHARACTER_SIZE = 8;

// Initialize text rendering
extern void Initialize();

// Shut down text rendering
extern void Shutdown();

// Draw a string
extern void DrawString(const std::string& text, PxVec2 position,
                       float scale = 1.0f, PxVec3 color = PxVec3(0),
                       PxVec2 box = PxVec2(GAME_WIDTH, GAME_HEIGHT),
                       bool cutOff = false,
                       PxVec2 padding = PxVec2(0, 2));
}; // namespace Text
