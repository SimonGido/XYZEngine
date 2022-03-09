#pragma once
#include "XYZ/Utils/DataStructures/CustomBuffer.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <box2d/box2d.h>

namespace XYZ {

    
	class ParticlePool
	{
	public:
        ParticlePool(const uint32_t maxParticles);
        ParticlePool(const ParticlePool& other);
        ParticlePool(ParticlePool&& other) noexcept;
        ~ParticlePool();

        ParticlePool& operator =(const ParticlePool& other);
        ParticlePool& operator =(ParticlePool&& other) noexcept;

        void SetMaxParticles(uint32_t maxParticles);
        void Wake(uint32_t id);
        void Kill(uint32_t id);

        // Default particle data
        struct Particle
        {
            glm::vec3 Position;
            glm::vec3 Velocity;
            
            glm::vec4 Color;
            glm::vec2 TexOffset;
            glm::vec3 Size;
            glm::quat Rotation;

            glm::vec3 LightColor;
            float     LightRadius;
            float     LightIntensity;

            float     LifeRemaining;
            bool      Alive;
        };

        Particle* Particles;

        uint32_t GetMaxParticles() const { return m_MaxParticles; }
        uint32_t GetAliveParticles() const { return m_AliveParticles; }
    private:
        void generateParticles(uint32_t particleCount);
        void deleteParticles();
        void swapData(uint32_t a, uint32_t b);
        void copyData(const ParticlePool& source);

    private:
        uint32_t m_MaxParticles;
        uint32_t m_AliveParticles;
    };
}