#pragma once
#include "XYZ/Core/Ref.h"
#include "ParticleData.h"

#include <mutex>

namespace XYZ {

	class ParticleGenerator : public RefCount
	{
	public:
		ParticleGenerator();

		virtual ~ParticleGenerator() = default;
		virtual void Generate(ParticleDataBuffer* data, uint32_t startId, uint32_t endId) const = 0;


		void SetEnabled(bool enabled);
		bool IsEnabled() const;

	protected:
		mutable std::mutex m_Mutex;

		bool		 m_Enabled;
	};

	enum class EmitShape
	{
		Box,
		Circle
	};

	class ParticleShapeGenerator : public ParticleGenerator
	{
	public:
		ParticleShapeGenerator();
		ParticleShapeGenerator(const ParticleShapeGenerator& other);
		ParticleShapeGenerator& operator=(const ParticleShapeGenerator& other);

		virtual void Generate(ParticleDataBuffer* data, uint32_t startId, uint32_t endId) const override;

		void SetEmitShape(EmitShape shape);
		void SetBoxMin(const glm::vec3& boxMin);
		void SetBoxMax(const glm::vec3& boxMax);
		void SetRadius(float radius);

		EmitShape GetEmitShape() const;
		glm::vec3 GetBoxMin() const;
		glm::vec3 GetBoxMax() const;
		float	  GetRadius() const;
	private:
		void generateBox(ParticleDataBuffer* data, uint32_t startId, uint32_t endId) const;
		void generateCircle(ParticleDataBuffer* data, uint32_t startId, uint32_t endId) const;

	private:
		EmitShape	 m_Shape;
		glm::vec3	 m_BoxMin;
		glm::vec3	 m_BoxMax;
		float		 m_Radius;	
	};

	class ParticleLifeGenerator : public ParticleGenerator
	{
	public:
		ParticleLifeGenerator();
		ParticleLifeGenerator(const ParticleLifeGenerator& other);
		ParticleLifeGenerator& operator =(const ParticleLifeGenerator& other);

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
		ParticleRandomVelocityGenerator(const ParticleRandomVelocityGenerator& other);
		ParticleRandomVelocityGenerator& operator=(const ParticleRandomVelocityGenerator& other);

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