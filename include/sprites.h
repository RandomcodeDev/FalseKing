#pragma once

#include "sprite.h"

namespace Sprites
{
// Load all sprites
void Load();

// Clean up sprites
void Unload();

namespace Player
{
extern Sprite player;
extern Sprite fireMelee;
} // namespace Player
}
