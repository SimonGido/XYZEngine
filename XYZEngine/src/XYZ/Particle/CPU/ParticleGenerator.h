#pragma once
#include "XYZ/Core/Ref.h"
#include "ParticleData.h"

#include <mutex>

namespace XYZ {

	class ParticleGenerator : public RefCount
	{
	public:
		virtual ~ParticleGenerator() = default;
		virtual void Generate(ParticleDataBuffer* data, uint32_t startId, uint32_t endId) const = 0;

	protected:
		mutable std::mutex m_Mutex;
	};

	enum class EmittShape
	{
		Box,
		Circle
	};

	class ParticleShapeGenerator : public ParticleGenerator
	{
	public:
		ParticleShapeGenerator();
		virtual void Generate(ParticleDataBuffer* data, uint32_t startId, uint32_t endId) const override;

		void SetBoxMin(const glm::vec3& boxMin);
		void SetBoxMax(const glm::vec3& boxMax);
		void SetRadius(float radius);

		glm::vec3 GetBoxMin() const;
		glm::vec3 GetBoxMax() const;
		float	  GetRadius() const;
	private:
		void generateBox(ParticleDataBuffer* data, uint32_t startId, uint32_t endId) const;
		void generateCircle(ParticleDataBuffer* data, uint32_t startId, uint32_t endId) const;

	private:
		EmittShape	 m_Shape;
		glm::vec3	 m_BoxMin;
		glm::vec3	 m_BoxMax;
		float		 m_Radius;
	};

	class ParticleBoxGenerator : public ParticleGenerator
	{
	public:
		ParticleBoxGenerator();

		virtual void Generate(ParticleDataBuffer* data, uint32_t startId, uint32_t endId) const override;
		
		void SetBoxMin(const glm::vec3& boxMin);
		void SetBoxMax(const glm::vec3& boxMax);

		glm::vec3 GetBoxMin() const;
		glm::vec3 GetBoxMax() const;
	private:
		glm::vec3 m_BoxMin;
		glm::vec3 m_BoxMax;
	};


	class ParticleCircleGenerator : public ParticleGenerator
	{
	public:
		ParticleCircleGenerator();

		virtual void Generate(ParticleDataBuffer* data, uint32_t startId, uint32_t endId) const override;
		
		void  SetRadius(float radius);
		float GetRadius() const;
	private:
		float m_Radius;
	};

	class ParticleLifeGenerator : public ParticleGenerator
	{
	public:
		ParticleLifeGenerator();

		virtual void Generate(ParticleDataBuffer* data, uint32_t startId, uint32_t endId) const override;

		void  SetLifeTime(float life);
		float GetLifeTime() const;
	private:
		float m_LifeTime;
	};


	class ParticleRandomVelocityGenerator : public ParticleGenerator
	{
	public:
		ParticleRandomVelocityGenerator();
		
		virtual void Generate(ParticleDataBuffer* data, uint32_t startId, uint32_t endId) const override;

		void SetMinVelocity(const glm::vec3& minVelocity);
		void SetMaxVelocity(const glm::vec3& maxVelocity);

		glm::vec3 GetMinVelocity() const;
		glm::vec3 GetMaxVelocity() const;

	private:
		glm::vec3 m_MinVelocity;
		glm::vec3 m_MaxVelocity;
	};
}