#pragma once
#include "XYZ/Core/Ref.h"
#include "ParticleDataBuffer.h"

#include <mutex>

namespace XYZ {

	enum class EmitShape
	{
		Box,
		Circle
	};

	class ParticleShapeGenerator
	{
	public:
		ParticleShapeGenerator();
		ParticleShapeGenerator(const ParticleShapeGenerator& other);
		ParticleShapeGenerator& operator=(const ParticleShapeGenerator& other);

		void Generate(ParticleDataBuffer& data, uint32_t startId, uint32_t endId) const;


		EmitShape	m_Shape;
		glm::vec3	m_BoxMin;
		glm::vec3	m_BoxMax;
		float		m_Radius;
		bool	    m_Enabled;

	private:
		void generateBox(ParticleDataBuffer& data, uint32_t startId, uint32_t endId) const;
		void generateCircle(ParticleDataBuffer& data, uint32_t startId, uint32_t endId) const;

	};

	class ParticleLifeGenerator
	{
	public:
		ParticleLifeGenerator();
		ParticleLifeGenerator(const ParticleLifeGenerator& other);
		ParticleLifeGenerator& operator =(const ParticleLifeGenerator& other);

		void  Generate(ParticleDataBuffer& data, uint32_t startId, uint32_t endId) const;

		float m_LifeTime;
		bool  m_Enabled;
	};


	class ParticleRandomVelocityGenerator
	{
	public:
		ParticleRandomVelocityGenerator();
		ParticleRandomVelocityGenerator(const ParticleRandomVelocityGenerator& other);
		ParticleRandomVelocityGenerator& operator=(const ParticleRandomVelocityGenerator& other);

		void Generate(ParticleDataBuffer& data, uint32_t startId, uint32_t endId) const;

		glm::vec3 m_MinVelocity;
		glm::vec3 m_MaxVelocity;
		bool	  m_Enabled;
	};
}