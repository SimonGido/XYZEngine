#pragma once

#include <glm/glm.hpp>

namespace XYZ {

	class ParticleDataBuffer
	{
	public:
        ParticleDataBuffer(uint32_t maxParticles);
        ~ParticleDataBuffer();

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
        glm::vec4*  m_TexCoord;
        glm::vec4*  m_StartColor;
        glm::vec4*  m_EndColor;
        glm::vec2*  m_Size;
        float*      m_Rotation;
        float*      m_AngularVelocity;
       

        uint32_t GetMaxParticles() const { return m_MaxParticles; }
        uint32_t GetAliveParticles() const { return m_AliveParticles; }
    private:
        void generateParticles(uint32_t particleCount);
        void swapData(uint32_t a, uint32_t b);

    private:
        uint32_t m_MaxParticles;
        uint32_t m_AliveParticles;
    };
}