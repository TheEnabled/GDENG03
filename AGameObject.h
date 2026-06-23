#pragma once
#include <vector>


struct Vertex
{
    float x,  y,  z;   // POSITION  (slot 0) — start position
    float x1, y1, z1; // POSITION  (slot 1) — end/target position for lerp animation
    float r,  g,  b;  // COLOR
};

class AGameObject
{
public:
    AGameObject(float x, float y, float z,
                float r, float g, float b)
        : m_x(x), m_y(y), m_z(z)
        , m_r(r), m_g(g), m_b(b)
    {
    }

    virtual ~AGameObject() = default;

    virtual void appendVertices(std::vector<Vertex>& outVerts) const = 0;

protected:
    float m_x, m_y, m_z;   // world position 
    float m_r, m_g, m_b;   // base color
};
