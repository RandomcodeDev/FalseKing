#include "physics.h"
#include "backend.h"
#include "camera.h"
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

#ifndef RETAIL
    m_scene->setVisualizationParameter(PxVisualizationParameter::eSCALE,
                                       100.0f);
    m_scene->setVisualizationParameter(
        PxVisualizationParameter::eCOLLISION_SHAPES, 1.0f);
#endif

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

void Visualize(flecs::iter& iter)
{
    Systems::Context* context = iter.ctx<Systems::Context>();

    const PxRenderBuffer& renderBuffer =
        context->physics->GetScene().getRenderBuffer();
    for (size_t i = 0; i < renderBuffer.getNbPoints(); i++)
    {
        auto& point = renderBuffer.getPoints()[i];
        if (context->mainCamera->IsVisible(point.pos, PxVec2(1, 1)))
        {
            g_backend->DrawPoint(context->mainCamera->Project(point.pos),
                                 UNPACK_COLOR(point.color));
        }
    }

    for (size_t i = 0; i < renderBuffer.getNbLines(); i++)
    {
        auto& line = renderBuffer.getLines()[i];
        if (context->mainCamera->IsVisible(line.pos0, PxVec2(1, 1)) ||
            context->mainCamera->IsVisible(line.pos1, PxVec2(1, 1)))
        {
            g_backend->DrawLine(context->mainCamera->Project(line.pos0),
                                context->mainCamera->Project(line.pos1),
                                UNPACK_COLOR(line.color0));
        }
    }
}

} // namespace Physics