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

void Core::Level::Shape::Write(std::ofstream& file) const
{
    // Sprites are basically like a reference to a region of an image, so
    // they're just 4 uint32_t's
    file.write((const char*)&sprite.x, sizeof(uint32_t));
    file.write((const char*)&sprite.y, sizeof(uint32_t));
    file.write((const char*)&sprite.width, sizeof(uint32_t));
    file.write((const char*)&sprite.height, sizeof(uint32_t));

    PxGeometryType::Enum type = shape->getGeometryType();
    file.write((const char*)&type, sizeof(uint32_t));
    switch (type)
    {
    case PxGeometryType::eCAPSULE:
        PxCapsuleGeometry capsule;
        shape->getCapsuleGeometry(capsule);
        file.write((const char*)&capsule.radius, sizeof(float));
        file.write((const char*)&capsule.halfHeight, sizeof(float));
        break;
    case PxGeometryType::eBOX:
        PxBoxGeometry box;
        shape->getBoxGeometry(box);
        file.write((const char*)&box.halfExtents.x, sizeof(float));
        file.write((const char*)&box.halfExtents.y, sizeof(float));
        file.write((const char*)&box.halfExtents.z, sizeof(float));
        break;
    }


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

void Core::Level::Save(const std::string& path) const
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

    uint64_t shapeCount = m_shapes.size();
    file.write((const char*)&shapeCount, sizeof(uint64_t));

    for (const auto& shape : m_shapes)
    {
        shape.Write(file);
    }
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
