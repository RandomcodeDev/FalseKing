#include "components.h"

CORE_API void Core::Components::Register(flecs::world& world)
{
    ecs_os_init();
}
