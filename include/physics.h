#pragma once

#include "game.h"

// Stores physics stuff
class PhysicsState
{
  public:
    static constexpr float GRAVITY = 9.8f;

    // Initialize physics stuff
    PhysicsState();

    // Shut down
    ~PhysicsState();

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

// Physics controller
struct PhysicsController
{
    PhysicsController(PhysicsState& physics, const PxControllerDesc& desc)
    {
        physics.GetControllerManager().createController(desc);
    };

    PxController& GetController()
    {
        return *m_controller;
    }

    PxTransform GetTransform()
    {
        return GetController().getActor()->getGlobalPose();
    }

  private:
    PxController* m_controller;
};