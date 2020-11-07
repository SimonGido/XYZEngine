#pragma once
#include "Camera.h"
#include "RenderPass.h"
#include "XYZ/Scene/Scene.h"

namespace XYZ {
	
	struct GridProperties
	{
		glm::mat4 Transform;
		glm::vec2 Scale;
		float LineWidth;
	};

	struct SceneRendererOptions
	{
		bool ShowGrid = true;
		bool ShowBoundingBoxes = false;
	};

	struct SceneRendererCamera
	{
		XYZ::Camera Camera;
		glm::mat4 ViewMatrix;
	};

	class SceneRenderer
	{
	public:
		static void Init();

		static void SetViewportSize(uint32_t width, uint32_t height);

		static void BeginScene(const Scene* scene, const SceneRendererCamera& camera);
		static void EndScene();

		static void SubmitSprite(SpriteRenderer* sprite, const glm::mat4& transform);
		static void SubmitParticles(ParticleComponent* particle, const glm::mat4& transform);
		static void SubmitLight(PointLight2D* light, const glm::mat4& transform);
		static void SetGridProperties(const GridProperties& props);

		static Ref<RenderPass> GetFinalRenderPass();
	
		static uint32_t GetFinalColorBufferRendererID();
		static SceneRendererOptions& GetOptions();
	private:
		static void FlushDrawList();
		static void GeometryPass();
		static void LightPass();
		static void BloomPass();
		static void GaussianBlurPass();
		static void CompositePass();
	};
}