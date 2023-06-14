#pragma once

#include "game.h"

// Scene implementation
class PhysicsScene : public PxScene
{
  public:
    virtual bool advance(PxReal delta);
};

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
    PxFoundation* m_foundation;
    PxPhysics* m_physics;
    PhysicsScene* m_scene;
};