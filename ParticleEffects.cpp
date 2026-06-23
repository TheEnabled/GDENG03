#include "ParticleEffects.h"
#include <random>

static float randf(float a, float b)
{
    static std::mt19937 gen((std::random_device())());
    std::uniform_real_distribution<float> d(a, b);
    return d(gen);
}

void ParticleEffects::explosion(ParticleSystem& system, const Vec3& pos, int count, float power)
{
    for (int i = 0; i < count; ++i)
    {
        Particle p;
        p.position = pos;
        // random direction
        p.velocity.x = randf(-1.0f, 1.0f);
        p.velocity.y = randf(-1.0f, 1.0f);
        p.velocity.z = randf(-1.0f, 1.0f);
        // normalize approx
        float len = std::sqrt(p.velocity.x*p.velocity.x + p.velocity.y*p.velocity.y + p.velocity.z*p.velocity.z) + 1e-6f;
        p.velocity.x = (p.velocity.x / len) * randf(0.1f, power);
        p.velocity.y = (p.velocity.y / len) * randf(0.1f, power);
        p.velocity.z = (p.velocity.z / len) * randf(0.1f, power);

        p.color = { randf(0.8f,1.0f), randf(0.4f,0.8f), randf(0.0f,0.2f), 1.0f };
        p.life_total = p.lifetime = randf(0.5f, 2.0f);
        p.alive = true;

        system.emit(p);
    }
}

void ParticleEffects::fountain(ParticleSystem& system, const Vec3& pos, int count, float spread)
{
    for (int i = 0; i < count; ++i)
    {
        Particle p;
        p.position = pos;
        p.velocity.x = randf(-spread, spread);
        p.velocity.y = randf(2.0f, 5.0f);
        p.velocity.z = randf(-spread, spread);

        p.color = { randf(0.4f,0.6f), randf(0.6f,0.8f), randf(0.9f,1.0f), 1.0f };
        p.life_total = p.lifetime = randf(1.0f, 3.0f);
        p.alive = true;

        system.emit(p);
    }
}

void ParticleEffects::rain(ParticleSystem& system, float point1, float point2, int count, float spread)
{
    for (int i = 0; i < count; i++)
    {
        Particle p;

        
        p.position.x = randf(point1, point2);
        p.position.y = 1.0f;                    // spawn at the top
        p.position.z = randf(point1, point2);

        // Falling straight down;
        p.velocity.x = randf(-0.05f, 0.05f);
        p.velocity.y = randf(-spread, -spread * 0.5f); // negative = downward
        p.velocity.z = randf(-0.05f, 0.05f);

        // Blue-tinted raindrop color
        p.color = { randf(0.4f, 0.6f), randf(0.4f, 0.6f), randf(0.9f, 1.0f), 1.0f };
        p.life_total = p.lifetime = randf(1.0f, 2.0f);
        p.alive = true;

        system.emit(p);
    }
}

