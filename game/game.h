#pragma once

#include "core/stdafx.h"

#ifdef GAME
#ifdef _WIN32
#define GAME_API ATTRIBUTE(dllexport)
#else
#define GAME_API ATTRIBUTE(visibility("default"))
#endif
#else
#ifdef _WIN32
#define GAME_API ATTRIBUTE(dllimport)
#else
#define GAME_API
#endif
#endif