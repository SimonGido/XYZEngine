#pragma once
#include "XYZ/Utils/DataStructures/CustomBuffer.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <box2d/box2d.h>

namespace XYZ {

    
	class ParticleDataBuffer
	{
	public:
        ParticleDataBuffer(uint32_t maxParticles);
        ParticleDataBuffer(const ParticleDataBuffer& other);
        ParticleDataBuffer(ParticleDataBuffer&& other) noexcept;
        ~ParticleDataBuffer();

        ParticleDataBuffer& operator =(const ParticleDataBuffer& other);
        ParticleDataBuffer& operator =(ParticleDataBuffer&& other) noexcept;

        void SetMaxParticles(uint32_t maxParticles);
        void Wake(uint32_t id);
        void Kill(uint32_t id);

        // Default particle data
        struct ParticleData
        {
            glm::vec4 Color;
            glm::vec3 Position;
            glm::vec3 Velocity;
            glm::vec2 TexOffset;
            glm::vec4 StartColor;
            glm::vec4 EndColor;
            glm::vec3 Size;
            glm::quat Rotation;
            float     LifeRemaining;
            bool      Alive;
        };

        ParticleData*   Particle;

        uint32_t GetMaxParticles() const { return m_MaxParticles; }
        uint32_t GetAliveParticles() const { return m_AliveParticles; }
    private:
        void generateParticles(uint32_t particleCount);
        void deleteParticles();
        void swapData(uint32_t a, uint32_t b);
        void copyData(const ParticleDataBuffer& source);

    private:
        uint32_t m_MaxParticles;
        uint32_t m_AliveParticles;
    };
}