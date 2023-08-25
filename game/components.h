#pragma once

#include "stdafx.h"

namespace Game
{
namespace Components
{
// Register game components
void Register(const flecs::world& world);

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
