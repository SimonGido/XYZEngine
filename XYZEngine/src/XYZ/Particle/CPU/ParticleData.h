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

        glm::vec3*  m_Position;
        glm::vec4*  m_Color;
        glm::vec4*  m_StartColor;
        glm::vec4*  m_EndColor;
        glm::vec3*  m_Velocity;
        float*      m_Rotation;
        float*      m_AngularVelocity;
        float*      m_LifeRemaining;
        bool*       m_Alive;

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