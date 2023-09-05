#include "image.h"
#include "physics.h"
#include "sprite.h"
#include "level.h"

Core::Level::Shape::Shape(Level& level, const Sprite& sprite, PxShape* shape)
{
    this->sprite = Sprite(level.m_spriteSheet, sprite.x, sprite.y, sprite.width,
                          sprite.height);
    this->shape = shape;
    this->shape->acquireReference();
}

Core::Level::Level(std::shared_ptr<Image> spriteSheet,
                   const std::vector<Shape>& shapes, Physics::State& physics)
    : m_spriteSheet(spriteSheet), m_shapes(shapes)
{
    m_shapes.reserve(shapes.size());
    m_body = Physics::Body(physics, PxTransform(), *shapes[0].shape);
    m_shapes.push_back(shapes[0]);
    std::for_each(shapes.begin() + 1, shapes.end(),
                  [&](const Shape& shape) { AddShape(shape); });
}

Core::Level::Level(const std::string& path, flecs::world& world)
{
}

void Core::Level::Save(const std::string& path)
{
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file.is_open())
    {
        Quit("Failed to open level file {}", path);
    }

    file.write((char*)&LEVEL_SIGNATURE, LEVEL_SIGNATURE_SIZE);
    if (m_spriteSheet->GetSource() != Image::MEMORY_PATH)
    {
        file.write(m_spriteSheet->GetSource().data(), m_spriteSheet->GetSource().size());
    }
    // NUL terminator for spritesheet path
    file.write(0, 1);
}

void Core::Level::AddShape(const Shape& shape)
{
    m_shapes.push_back(shape);
    m_body.GetBody().attachShape(*m_shapes.end()->shape);
}

void Core::Level::AddShapes(const std::vector<Shape>& shapes)
{
    m_shapes.resize(m_shapes.size() + shapes.size());
    std::copy(shapes.begin(), shapes.end(), m_shapes.end() - shapes.size());
    std::for_each(m_shapes.end() - shapes.size(), m_shapes.end(),
                  [&](const Shape& shape) {
                      m_body.GetBody().attachShape(*shape.shape);
                  });
}
