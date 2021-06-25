#pragma once

#include "XYZ/Renderer/RendererCommand.h"
#include "XYZ/Core/Ref.h"

#include <glm/glm.hpp>

namespace XYZ {
	struct ParticleRenderData
	{
		glm::vec4 Color;
		glm::vec4 TexCoord;
		glm::vec2 Position;
		glm::vec2 Size;
		float	  Rotation;
	};

	struct ParticleRendererCPU : public RendererCommand
	{
		ParticleRendererCPU(uint32_t maxParticles);

		virtual void Bind() const override;

	private:
		Ref<VertexArray>			    VAO;
		Ref<VertexBuffer>			    InstanceVBO;
		uint32_t						InstanceCount;

		friend class ParticleSystemCPU;
	};

}