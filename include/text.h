#pragma once

#include "backend.h"
#include "game.h"
#include "image.h"

namespace Text
{
// Character size
constexpr uint32_t CHARACTER_SIZE = 8;

// Initialize text rendering
extern void Initialize(Backend* backend);

// Shut down text rendering
extern void Shutdown(Backend* backend);

// Draw a string
extern void DrawString(Backend* backend, const std::string& text,
                       glm::uvec2 position, float scale = 1.0f,
                       glm::uvec2 box = glm::uvec2(GAME_WIDTH, GAME_HEIGHT),
                       bool cutOff = false,
                       glm::uvec2 padding = glm::uvec2(0, 2),
                       glm::u8vec4 color = glm::u8vec4(0));
}; // namespace Text
