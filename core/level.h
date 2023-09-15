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
        friend class Level;

        Sprite sprite;
        PxShape* shape;

        Shape() = default;
        Shape(Level& level, const Sprite& sprite, PxShape* shape);

      protected:
        Shape(std::ofstream& file);
        void Write(std::ofstream& file) const;
    };

    // 'FKLEVEL\0' in little endian
    static constexpr uint64_t LEVEL_SIGNATURE = 0x0076698669767570;
    static constexpr size_t LEVEL_SIGNATURE_SIZE = 8;

    // Level format
    //
    // 0: LEVEL_SIGNATURE
    // 8: sprite sheet path
    // 8 + path length: NUL terminator
    // 9 + path length: shape count
    // 17 + path length: shapes
    // 17 + path length + shapes size: entity data

    // Create a level
    Level(std::shared_ptr<Image> spriteSheet, const std::vector<Shape>& shapes,
          Physics::State& state);

    // Load a level
    Level(const std::string& path, flecs::world& world);

    // Save a level
    void Save(const std::string& path) const;

    // Add a shape
    void AddShape(const Shape& shape);

    // Add shapes
    void AddShapes(const std::vector<Shape>& shapes);

  private:
    std::shared_ptr<Image> m_spriteSheet;
    std::vector<Shape> m_shapes;
    Physics::Body m_body;
};

} // namespace Core
