#pragma once

#include "stdafx.h"

namespace Components
{

class Camera
{
  public:
    PxVec2 position;
    bool followOwner;

    Camera() : position(PxVec2(0, 0)), followOwner(true){};
    Camera(const Camera& other) = default;
    Camera(PxVec2 position, bool followOwner)
        : position(position), followOwner(followOwner)
    {
    }

    // Project a 3D position in space to a 2D position on screen
    PxVec2 Project(const PxVec3& position) const;

    // Decides if a position is visible on screen
    bool IsVisible(const PxVec3& position, const PxVec2& size) const;
};

} // namespace Components

namespace Systems
{

// Track entities with cameras
void CameraTrack(flecs::entity entity, Components::Camera& camera);

} // namespace Systems
