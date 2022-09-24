#pragma once
#include "XYZ/Renderer/Renderer.h"
#include "XYZ/Debug/Profiler.h"
#include "XYZ/Scene/Scene.h"

namespace XYZ {

	struct DeferredLightPassConfiguration
	{
		Ref<RenderPass> Pass;
		Ref<Shader>		Shader;
	};

	class DeferredLightPass
	{
	public:
		void Init(const DeferredLightPassConfiguration& config, const Ref<RenderCommandBuffer>& commandBuffer);

		void PreSubmit(Ref<Scene> scene);

		void Submit(
			const Ref<RenderCommandBuffer>& commandBuffer, 
			const Ref<Image2D>& colorImage,
			const Ref<Image2D>& positionImage
		);

		static constexpr uint32_t GetMaxNumberOfLights() { return sc_MaxNumberOfLights; }
	private:
		void createPipeline();

	private:
		struct PointLight
		{
			glm::vec4 Color;
			glm::vec2 Position;
			float	  Radius;
			float	  Intensity;
		};
		struct SpotLight
		{
			glm::vec4 Color;
			glm::vec2 Position;
			float	  Radius;
			float	  Intensity;
			float	  InnerAngle;
			float	  OuterAngle;

			float Alignment[2];
		};


		Ref<RenderPass>       m_RenderPass;
		Ref<UniformBufferSet> m_CameraBufferSet;
		Ref<StorageBufferSet> m_LightStorageBufferSet;

		Ref<Shader>			  m_Shader;
		Ref<Pipeline>		  m_Pipeline;
		Ref<Material>		  m_Material;
		Ref<MaterialInstance> m_MaterialInstance;


		std::vector<PointLight>	m_PointLights;
		std::vector<SpotLight>	m_SpotLights;

		static constexpr uint32_t sc_MaxNumberOfLights = 2 * 1024;
	};
}