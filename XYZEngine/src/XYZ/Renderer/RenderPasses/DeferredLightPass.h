#pragma once
#include "XYZ/Renderer/Renderer.h"

#include "XYZ/Scene/Scene.h"
#include "XYZ/Scene/Prefab.h"

namespace XYZ {

	struct DeferredLightPassConfiguration
	{
		Ref<RenderPass> Pass;
		Ref<Shader>		Shader;
	};

	struct DeferredLightPassStatistics
	{
		uint32_t PointLightCount;
		uint32_t SpotLightCount;
	};

	class XYZ_API DeferredLightPass
	{
	public:
		void Init(const DeferredLightPassConfiguration& config, const Ref<RenderCommandBuffer>& commandBuffer);

		DeferredLightPassStatistics PreSubmit(Ref<Scene> scene);

		void Submit(
			const Ref<RenderCommandBuffer>& commandBuffer, 
			const Ref<Image2D>& colorImage,
			const Ref<Image2D>& positionImage
		);

		static constexpr uint32_t GetMaxNumberOfLights() { return sc_MaxNumberOfLights; }
	private:
		void createPipeline();

	private:
		Ref<RenderPass>       m_RenderPass;
		Ref<UniformBufferSet> m_CameraBufferSet;
		Ref<StorageBufferSet> m_LightStorageBufferSet;

		Ref<Shader>			  m_Shader;
		Ref<Pipeline>		  m_Pipeline;
		Ref<Material>		  m_Material;
		Ref<MaterialInstance> m_MaterialInstance;


		static constexpr uint32_t sc_MaxNumberOfLights = 2 * 1024;
	};
}