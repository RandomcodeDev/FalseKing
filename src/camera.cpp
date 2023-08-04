#include "camera.h"
#include "physics.h"

PxVec2 Components::Camera::Project(const PxVec3& position) const
{
    // TODO: Should this be assuming top-left coordinates?
    // TODO: Center the owner properly
    float x = position.x - this->position.x + GAME_WIDTH / 2.5f;
    float y = position.z - position.y + this->position.y + GAME_HEIGHT / 2.5f;

    return PxVec2(x, y);
}

bool Components::Camera::IsVisible(const PxVec3& position,
                                   const PxVec2& size) const
{
    PxVec2 screenPosition = Project(position);

    // Basically, is any part of the given area within the screen
    float x = screenPosition.x + size.x;
    float y = screenPosition.y + size.y;
    return true; //(0 < x) && (x < GAME_WIDTH) && (0 < y) && (y < GAME_HEIGHT);
}

void Systems::CameraTrack(flecs::entity entity, Components::Camera& camera)
{
    auto object = Physics::GetBase(entity);

    if (camera.followOwner)
    {
        camera.position.x = object->GetTransform().p.x;
        camera.position.y =
            object->GetTransform().p.y - object->GetTransform().p.z;
    }
}
