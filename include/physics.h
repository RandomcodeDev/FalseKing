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
    virtual PxTransform GetTransform() const
    {
        return PxTransform();
    }

    virtual const PxShape* GetShape(uint32_t index) const
    {
        return nullptr;
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

    PxTransform GetTransform() const
    {
        return m_body->getGlobalPose();
    }

    const PxShape* GetShape(uint32_t index) const
    {
        std::vector<PxShape*> shapes(m_body->getNbShapes());
        if (index >= shapes.size())
        {
            return nullptr;
        }

        m_body->getShapes(shapes.data(), (uint32_t)shapes.size());

        return shapes[index];
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

    PxController& GetController() const
    {
        return *m_controller;
    }

    PxTransform GetTransform() const
    {
        return GetController().getActor()->getGlobalPose();
    }

    const PxShape* GetShape(uint32_t index) const
    {
        std::vector<PxShape*> shapes(GetController().getActor()->getNbShapes());
        if (index >= shapes.size())
        {
            return nullptr;
        }
        
        GetController().getActor()->getShapes(shapes.data(), (uint32_t)shapes.size());

        return shapes[index];
    }

  private:
    PxController* m_controller;
};

} // namespace Physics