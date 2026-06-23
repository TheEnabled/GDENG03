#pragma once
#include "ParticleSystem.h"
#include <random>

class ParticleEffects
{
public:
    // Create an explosion at position 'pos' by emitting many particles
    static void explosion(ParticleSystem& system, const Vec3& pos, int count, float power);

    // Create a fountain effect at position 'pos' that emits particles upward
    static void fountain(ParticleSystem& system, const Vec3& pos, int count, float spread);

	//Create a rain effect at position 'pos' that emits particles downward
    static void rain(ParticleSystem& system, float point1, float point2, int count, float spread);
};
