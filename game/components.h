#pragma once

#include "game.h"

namespace Game
{
namespace Components
{
// Register game components
extern GAME_API void Register(flecs::world& world);

// Element
struct Element
{
    enum
    {
        None,
        Fire,
        Water,
        Air,
        Earth,
        Ultimate
    } value;
};
} // namespace Components
} // namespace Game
