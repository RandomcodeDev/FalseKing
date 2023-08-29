#pragma once

#include "stdafx.h"

namespace Core
{
namespace Discord
{
constexpr uint64_t APP_ID = 1124873777105346621;
constexpr chrono::milliseconds API_COOLDOWN = chrono::milliseconds(16);
constexpr chrono::milliseconds ACTIVITY_COOLDOWN =
    chrono::milliseconds(4000); // 5/20 seconds

// Initialize the Discord SDK
extern CORE_API void Initialize();

// Update the status
extern CORE_API void Update(chrono::seconds runtime, chrono::milliseconds delta);

// Shut down the Discord SDK
extern CORE_API void Shutdown();

// Whether Discord is available
extern CORE_API bool Available();

// Whether Discord is connected
extern CORE_API bool Connected();
} // namespace Discord
} // namespace Core
