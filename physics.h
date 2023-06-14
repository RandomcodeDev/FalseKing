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

  private:
    physx::PxFoundation* m_foundation;
    physx::PxPhysics* m_physics;
};