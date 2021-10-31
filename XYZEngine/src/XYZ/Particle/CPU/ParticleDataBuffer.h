#pragma once
#include "XYZ/Utils/DataStructures/CustomBuffer.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

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
        struct Particle
        {
            glm::vec4 Color;
            glm::vec3 Position;
            glm::vec3 Velocity;
           
            float     LifeRemaining;
            bool      Alive;
        };

        Particle*   m_Particle;

        // Additional particle properties;
        glm::vec2*  m_TexOffset;
        glm::vec4*  m_StartColor;
        glm::vec4*  m_EndColor;
        glm::vec3*  m_Size;
        glm::vec3*  m_Lights;
        glm::quat*  m_Rotation;

        CustomBuffer m_CustomData[3];

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