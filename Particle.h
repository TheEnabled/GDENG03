#pragma once
#include <cstdint>

// Simple math types used by the particle system
struct Vec3
{
    float x{0.0f};
    float y{0.0f};
    float z{0.0f};
};

struct Color
{
    float r{1.0f};
    float g{1.0f};
    float b{1.0f};
    float a{1.0f};
};

struct Particle
{
    Vec3 position;
    Vec3 velocity;
    Color color;
    float lifetime{0.0f}; // remaining life in seconds
    float life_total{0.0f}; // total lifespan
    bool alive{false};
};

