#pragma once

#include "core/sprite.h"

namespace Game
{
namespace Sprites
{
// Load all sprites
extern GAME_API void Load();

// Clean up sprites
extern GAME_API void Unload();

namespace Player
{
extern GAME_API Core::Sprite player;
extern GAME_API Core::Sprite cursor;
extern GAME_API Core::Sprite fireMelee;
} // namespace Player
}
}
