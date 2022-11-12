#pragma once
#include "XYZ/Renderer/Renderer.h"
#include "XYZ/Debug/Profiler.h"

namespace XYZ {

	struct BloomPassConfiguration
	{
		Ref<Shader>					  Shader;
		std::array<Ref<Texture2D>, 3> BloomTexture;
	};

	struct BloomSettings
	{
		float FilterTreshold = 1.0f;
		float FilterKnee = 0.1f;
	};

	class XYZ_API BloomPass
	{
	public:
		void Init(const BloomPassConfiguration& config, const Ref<RenderCommandBuffer>& commandBuffer);

		void SetBloomTextures(const std::array<Ref<Texture2D>, 3>& textures);

		void Submit(
			const Ref<RenderCommandBuffer>& commandBuffer, 
			const Ref<Image2D>& lightImage, 
			const BloomSettings& bloomSettings, 
			glm::ivec2 viewportSize
		);


	private:
		Ref<Material>				  m_BloomComputeMaterial;
		Ref<MaterialInstance>		  m_BloomComputeMaterialInstance;
		std::array<Ref<Texture2D>, 3> m_BloomTexture;
		Ref<PipelineCompute>		  m_BloomComputePipeline;
	};
}