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

		void SetCustomVBOLayout(const BufferLayout& layout);
	private:
		void createBuffers();

	private:
		Ref<VertexArray>			    m_VAO;
		Ref<VertexBuffer>			    m_InstanceVBO;
		Ref<VertexBuffer>				m_CustomVBO;
		uint32_t						m_InstanceCount;
		uint32_t						m_MaxParticles;

		friend class ParticleSystemCPU;
	};

}