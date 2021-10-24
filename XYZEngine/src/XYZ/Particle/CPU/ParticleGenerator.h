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

		void Generate(ParticleDataBuffer* data, uint32_t startId, uint32_t endId) const;

		void SetEnabled(bool enabled) { m_Enabled = enabled; }
		void SetEmitShape(EmitShape shape);
		void SetBoxMin(const glm::vec3& boxMin);
		void SetBoxMax(const glm::vec3& boxMax);
		void SetRadius(float radius);

		EmitShape GetEmitShape() const;
		glm::vec3 GetBoxMin() const;
		glm::vec3 GetBoxMax() const;
		float	  GetRadius() const;

		bool      IsEnabled() const { return m_Enabled; }
	private:
		void generateBox(ParticleDataBuffer* data, uint32_t startId, uint32_t endId) const;
		void generateCircle(ParticleDataBuffer* data, uint32_t startId, uint32_t endId) const;

	private:
		EmitShape	m_Shape;
		glm::vec3	m_BoxMin;
		glm::vec3	m_BoxMax;
		float		m_Radius;	
		bool	    m_Enabled;
	};

	class ParticleLifeGenerator
	{
	public:
		ParticleLifeGenerator();
		ParticleLifeGenerator(const ParticleLifeGenerator& other);
		ParticleLifeGenerator& operator =(const ParticleLifeGenerator& other);

		void  Generate(ParticleDataBuffer* data, uint32_t startId, uint32_t endId) const;
		void  SetEnabled(bool enabled) { m_Enabled = enabled; }
		void  SetLifeTime(float life);
		float GetLifeTime() const;
		bool  IsEnabled() const { return m_Enabled; }
	private:
		float m_LifeTime;
		bool  m_Enabled;
	};


	class ParticleRandomVelocityGenerator
	{
	public:
		ParticleRandomVelocityGenerator();
		ParticleRandomVelocityGenerator(const ParticleRandomVelocityGenerator& other);
		ParticleRandomVelocityGenerator& operator=(const ParticleRandomVelocityGenerator& other);

		void Generate(ParticleDataBuffer* data, uint32_t startId, uint32_t endId) const;
		void SetEnabled(bool enabled) { m_Enabled = enabled; }
		void SetMinVelocity(const glm::vec3& minVelocity);
		void SetMaxVelocity(const glm::vec3& maxVelocity);

		glm::vec3 GetMinVelocity() const;
		glm::vec3 GetMaxVelocity() const;
		bool      IsEnabled() const { return m_Enabled; }
	private:
		glm::vec3 m_MinVelocity;
		glm::vec3 m_MaxVelocity;
		bool	  m_Enabled;
	};
}