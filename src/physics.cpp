#include "physics.h"
#include "systems.h"

namespace Physics
{

class ErrorCallback : public PxErrorCallback
{
  public:
    virtual void reportError(PxErrorCode::Enum code, const char* message,
                             const char* file, int line)
    {
        QUIT_CODE(code, "PhysX error at {}:{}: {}", file, line, message);
    }
};

static ErrorCallback g_physxErrorCallback;
static PxDefaultAllocator g_physxAllocator;

State::State()
{
    SPDLOG_INFO("Initializing physics");

    m_foundation = PxCreateFoundation(PX_PHYSICS_VERSION, g_physxAllocator,
                                      g_physxErrorCallback);
    if (!m_foundation)
    {
        Quit("PxCreateFoundation failed");
    }

    m_physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_foundation,
                                PxTolerancesScale(1, 1));
    if (!m_physics)
    {
        Quit("PxCreatePhysics failed");
    }

    m_dispatcher = PxDefaultCpuDispatcherCreate(2);

    PxSceneDesc sceneDesc = PxSceneDesc(m_physics->getTolerancesScale());
    sceneDesc.gravity = PxVec3(0.0f, -GRAVITY, 0.0f);
    sceneDesc.cpuDispatcher = m_dispatcher;
    sceneDesc.filterShader = PxDefaultSimulationFilterShader;
    m_scene = m_physics->createScene(sceneDesc);

    m_controllerManager = PxCreateControllerManager(*m_scene);

    SPDLOG_INFO("Physics initialized");
}

State::~State()
{
    SPDLOG_INFO("Shutting down physics");
    m_controllerManager->release();
    m_scene->release();
    m_physics->release();
    m_foundation->release();
    SPDLOG_INFO("Physics shut down");
}

void State::Update(float delta)
{
    m_scene->simulate(delta);
    m_scene->fetchResults(true);
}

void Update(flecs::iter& iter)
{
    Systems::Context* context = iter.ctx<Systems::Context>();
    context->physics->Update(iter.delta_system_time());
}

} // namespace Physics