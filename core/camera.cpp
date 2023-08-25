#include "camera.h"
#include "physics.h"

PxVec2 Core::Components::Camera::Project(const PxVec3& position) const
{
    // TODO: Should this be assuming top-left coordinates?
    float x = position.x - this->position.x + GAME_WIDTH / 2.5f;
    float y = position.z - position.y + this->position.y + GAME_HEIGHT / 2.5f;

    return PxVec2(x, y);
}

bool Core::Components::Camera::IsVisible(const PxVec3& position,
                                   const PxVec2& size) const
{
    PxVec2 screenPosition = Project(position);

    float x = screenPosition.x + size.x;
    float y = screenPosition.y + size.y;

    // Basically, is any part of the given area within the screen
    return (0 < x) && (x < GAME_WIDTH) && (0 < y) && (y < GAME_HEIGHT);
}

void Core::Systems::CameraTrack(flecs::entity entity,
                                Core::Components::Camera& camera)
{
    auto object = Physics::GetBase(entity);

    if (camera.followOwner)
    {
        camera.position.x = object->GetTransform().p.x;
        camera.position.y = (object->GetTransform().p.y *
                             (1.0f / Core::Components::CAMERA_ANGLE)) -
            object->GetTransform().p.z;
    }
}
