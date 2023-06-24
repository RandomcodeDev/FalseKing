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
extern void DrawString(const std::string& text, glm::uvec2 position,
                       float scale = 1.0f, glm::u8vec3 color = glm::u8vec3(0),
                       glm::uvec2 box = glm::uvec2(GAME_WIDTH, GAME_HEIGHT),
                       bool cutOff = false,
                       glm::uvec2 padding = glm::uvec2(0, 2));
}; // namespace Text
