#pragma once

#include "stdafx.h"

namespace Discord
{
constexpr uint64_t APP_ID = 1124873777105346621;
constexpr chrono::milliseconds API_COOLDOWN = chrono::milliseconds(16);
constexpr chrono::milliseconds ACTIVITY_COOLDOWN =
    chrono::milliseconds(4000); // 5/20 seconds

// Initialize the Discord SDK
void Initialize();

// Update the status
void Update(chrono::seconds runtime, chrono::milliseconds delta);

// Shut down the Discord SDK
void Shutdown();

// Whether Discord is available
bool Available();

// Whether Discord is connected
bool Connected();
} // namespace Discord
