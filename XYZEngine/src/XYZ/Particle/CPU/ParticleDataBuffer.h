#pragma once

#include <glm/glm.hpp>

namespace XYZ {

    class ParticleCustomData
    {
    public:
        ParticleCustomData();
        ParticleCustomData(size_t elementSize, size_t size);
        ~ParticleCustomData();
       
        void Resize(size_t size);
        void SetElementSize(size_t size);
        void Clear();
      
        template <typename T>
        T* As();

        template <typename T>
        const T* As() const;

        size_t Size() const { return m_Size; }
    private:
        std::byte* m_Data;
        size_t     m_ElementSize;
        size_t	   m_Size;
    };

    template <typename T>
    T* ParticleCustomData::As()
    {
        XYZ_ASSERT(m_ElementSize == sizeof(T), "Different element size");
        return reinterpret_cast<T*>(m_Data);
    }

    template <typename T>
    const T* ParticleCustomData::As() const
    {
        XYZ_ASSERT(m_ElementSize == sizeof(T), "Different element size");
        return reinterpret_cast<T*>(m_Data);
    }

	class ParticleDataBuffer
	{
	public:
        ParticleDataBuffer(uint32_t maxParticles);
        ParticleDataBuffer(ParticleDataBuffer&& other) noexcept;
        ~ParticleDataBuffer();

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
        glm::vec4*  m_TexCoord;
        glm::vec4*  m_StartColor;
        glm::vec4*  m_EndColor;
        glm::vec2*  m_Size;
        glm::vec3*  m_Lights;

        float*      m_Rotation;
        float*      m_AngularVelocity;
       
        ParticleCustomData m_CustomData[3];

        uint32_t GetMaxParticles() const { return m_MaxParticles; }
        uint32_t GetAliveParticles() const { return m_AliveParticles; }
    private:
        void generateParticles(uint32_t particleCount);
        void deleteParticles();
        void swapData(uint32_t a, uint32_t b);


    private:
        uint32_t m_MaxParticles;
        uint32_t m_AliveParticles;
    };
}