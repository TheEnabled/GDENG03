#include "ParticleSystem.h"
#include <algorithm>

ParticleSystem::ParticleSystem(size_t max_particles)
    : m_max_particles(max_particles)
{
    m_particles.reserve(m_max_particles);
}

ParticleSystem::~ParticleSystem()
{
    clear();
}

void ParticleSystem::emit(const Particle& p)
{
    // try to find a dead particle to reuse
    for (auto &part : m_particles)
    {
        if (!part.alive)
        {
            part = p;
            return;
        }
    }

    if (m_particles.size() < m_max_particles)
    {
        m_particles.push_back(p);
    }
    // otherwise drop the particle
}

void ParticleSystem::update(float dt)
{
    for (auto &p : m_particles)
    {
        if (!p.alive) continue;

        // integrate position
        p.position.x += p.velocity.x * dt;
        p.position.y += p.velocity.y * dt;
        p.position.z += p.velocity.z * dt;

        // decrease lifetime
        p.lifetime -= dt;
        if (p.lifetime <= 0.0f)
        {
            p.alive = false;
        }
        else
        {
            // simple fade out based on remaining lifetime
            float t = p.lifetime / p.life_total;
            p.color.a = t;
        }
    }
}

void ParticleSystem::clear()
{
    m_particles.clear();
}

