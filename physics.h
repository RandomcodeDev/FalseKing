#pragma once

#include "game.h"

// Stores physics stuff
class PhysicsState
{
  public:
    // Initialize physics stuff
    PhysicsState();

    // Shut down
    ~PhysicsState();

    // Update state
    void Update(chrono::milliseconds delta);

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

  private:
    static constexpr float GRAVITY = 9.8f;

    PxFoundation* m_foundation;
    PxPhysics* m_physics;
    PxCpuDispatcher* m_dispatcher;
    PxScene* m_scene;
};