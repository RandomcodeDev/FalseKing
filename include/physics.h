#pragma once

#include "game.h"

namespace Physics
{
static constexpr float GRAVITY = 9.8f;
static constexpr float TIME_STEP = 1 / 60.0f;

// Stores physics stuff
class State
{
  public:
    // Initialize physics stuff
    State();

    // Shut down
    ~State();

    // Update state
    void Update(float delta);

    // Get physics
    PxPhysics& GetPhysics()
    {
        return *m_physics;
    }

    // Get scene
    PxScene& GetScene()
    {
        return *m_scene;
    }

    // Get controller manager
    PxControllerManager& GetControllerManager()
    {
        return *m_controllerManager;
    }

  private:
    PxFoundation* m_foundation;
    PxPhysics* m_physics;
    PxCpuDispatcher* m_dispatcher;
    PxScene* m_scene;
    PxControllerManager* m_controllerManager;
};

void Update(flecs::iter& iter);

// For systems that just need basic information like transform
struct Base
{
    virtual PxTransform& GetTransform()
    {
        return PxTransform();
    }
};

// Physics body
struct Body : Base
{
    Body() = default;
    Body(State& physics, const PxTransform& transform, PxShape& shape)
    {
        m_body = physics.GetPhysics().createRigidDynamic(transform);
        m_body->attachShape(shape);
        shape.release();
        physics.GetScene().addActor(*m_body);
    }

    PxRigidDynamic& GetBody()
    {
        return *m_body;
    }

    PxTransform& GetTransform()
    {
        return m_body->getGlobalPose();
    }

  private:
    PxRigidDynamic* m_body;
};

// Physics controller
struct Controller : Base
{
    Controller() = default;
    Controller(State& physics, const PxControllerDesc& desc)
    {
        m_controller = physics.GetControllerManager().createController(desc);
    };

    PxController& GetController()
    {
        return *m_controller;
    }

    PxTransform& GetTransform()
    {
        return GetController().getActor()->getGlobalPose();
    }

  private:
    PxController* m_controller;
};

} // namespace Physics