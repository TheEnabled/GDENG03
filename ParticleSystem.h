#pragma once
#include <vector>
#include <cstddef>
#include "Particle.h"

class ParticleSystem
{
public:
    ParticleSystem(size_t max_particles = 1000);
    ~ParticleSystem();

    // Emit a single particle with specified properties
    void emit(const Particle& p);

    // Update all particles by dt (seconds)
    void update(float dt);

    // Clear all particles
    void clear();

    // Access to particle list (read-only)
    const std::vector<Particle>& particles() const { return m_particles; }

private:
    std::vector<Particle> m_particles;
    size_t m_max_particles;
};


