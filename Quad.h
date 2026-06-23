#pragma once
#include "AGameObject.h"

class Quad : public AGameObject
{
public:
   
    Quad(float x, float y, float z,
         float r, float g, float b,
         float halfSize = 0.2f)
        : AGameObject(x, y, z, r, g, b)
        , m_halfSize(halfSize)
    {
    }

    void appendVertices(std::vector<Vertex>& out) const override
    {
        // Position 0: original quad corners
        float left   = m_x - m_halfSize;
        float right  = m_x + m_halfSize;
        float top    = m_y + m_halfSize;
        float bottom = m_y - m_halfSize;

        // Position 1: target shape — squished horizontally (half width, double height)
        // The vertex shader lerps between pos0 and pos1 using sin(m_time)
        float left1   = m_x - m_halfSize * 0.5f;
        float right1  = m_x + m_halfSize * 0.5f;
        float top1    = m_y + m_halfSize * 2.0f;
        float bottom1 = m_y - m_halfSize * 2.0f;

        // Triangle 1 — top-left, top-right, bottom-left
        //              { pos0 (xyz),          pos1 (xyz),            color (rgb) }
        out.push_back({ left,  top,    m_z,   left1,  top1,    m_z,  m_r, m_g, m_b });
        out.push_back({ right, top,    m_z,   right1, top1,    m_z,  m_r, m_g, m_b });
        out.push_back({ left,  bottom, m_z,   left1,  bottom1, m_z,  m_r, m_g, m_b });

        // Triangle 2 — top-right, bottom-right, bottom-left
        out.push_back({ right, top,    m_z,   right1, top1,    m_z,  m_r, m_g, m_b });
        out.push_back({ right, bottom, m_z,   right1, bottom1, m_z,  m_r, m_g, m_b });
        out.push_back({ left,  bottom, m_z,   left1,  bottom1, m_z,  m_r, m_g, m_b });
    }

private:
    float m_halfSize;
};
