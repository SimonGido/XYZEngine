#pragma once
#include "XYZ/Renderer/VertexArray.h"
#include "XYZ/Renderer/Material.h"
#include "XYZ/Core/Ref.h"
#include "XYZ/Core/Timestep.h"

#include "ParticleMaterial.h"
#include "Particle.h"

#include <glm/glm.hpp>

namespace XYZ {
	class ParticleSystem : public RefCount
	{
	public:
		ParticleSystem(const Ref<ParticleMaterial>& material);

		

		const Ref<ParticleMaterial>& GetMaterial() const { return m_Material; }
	private:
		Ref<ParticleMaterial> m_Material;
	};

}