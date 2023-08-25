#pragma once

#include "sprite.h"

namespace Game
{
namespace Sprites
{
// Load all sprites
void Load();

// Clean up sprites
void Unload();

namespace Player
{
extern Sprite player;
extern Sprite cursor;
extern Sprite fireMelee;
} // namespace Player
}
}
