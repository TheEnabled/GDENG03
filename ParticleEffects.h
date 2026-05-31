#pragma once
#include "ParticleSystem.h"
#include <random>

class ParticleEffects
{
public:
    // Create an explosion at position 'pos' by emitting many particles
    static void explosion(ParticleSystem& system, const Vec3& pos, int count = 100, float power = 5.0f);

    // Create a fountain effect at position 'pos' that emits particles upward
    static void fountain(ParticleSystem& system, const Vec3& pos, int count = 50, float spread = 1.0f);

	//Create a rain effect at position 'pos' that emits particles downward
    //static void rain(ParticleSystem& system, const Vec3& pos, int count = 50, float spread = 1.0f);
};


