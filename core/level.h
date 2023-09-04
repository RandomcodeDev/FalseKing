#pragma once

#include "stdafx.h"

namespace Core
{

class Image;
class Sprite;

namespace Physics
{
class Body;
}

class Level
{
  public:
    struct Shape
    {
        Sprite sprite;
        PxShape* shape;

        Shape(const Level& level, const Sprite& sprite, PxShape* shape);
    };

    // 'FKLEVEL\0' in little endian
    static constexpr uint64_t LEVEL_SIGNATURE = 0x0076698669767570;

    // Create a level
    Level(const Image& spriteSheet, const std::vector<Sprite>& sprites,
          const Physics::Body& body);

    // Load a level
    Level(const std::string& path);

    // Add a shape
    void AddShape(const Shape& shape, const Sprite& sprite);

    // Add shapes
    void AddShapes(const std::vector<Shape>& shapes, const Sprite& sprite);

  private:
    Image m_spriteSheet;
    std::vector<Shape> m_shapes;
    Physics::Body m_body;
};

} // namespace Core
