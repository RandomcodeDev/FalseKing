#include "physics.h"

class ErrorCallback : public PxErrorCallback
{
  public:
    virtual void reportError(PxErrorCode::Enum code, const char* message,
                             const char* file, int line)
    {
        Quit(fmt::format("PhysX error at {}:{}: {}", file, line, message),
             code);
    }
};

static ErrorCallback g_physxErrorCallback;
static PxDefaultAllocator g_physxAllocator;

PhysicsState::PhysicsState()
{
    SPDLOG_INFO("Initializing physics");

    m_foundation = PxCreateFoundation(PX_PHYSICS_VERSION, g_physxAllocator,
                                      g_physxErrorCallback);
    if (!m_foundation)
    {
        Quit("PxCreateFoundation failed");
    }

    m_physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_foundation,
                                PxTolerancesScale());
    if (!m_physics)
    {
        Quit("PxCreatePhysics failed");
    }

    SPDLOG_INFO("Physics initialized");
}

PhysicsState::~PhysicsState()
{
    SPDLOG_INFO("Shutting down physics");
    m_physics->release();
    m_foundation->release();
    SPDLOG_INFO("Physics shut down");
}

void PhysicsState::Update(chrono::milliseconds delta)
{
}
