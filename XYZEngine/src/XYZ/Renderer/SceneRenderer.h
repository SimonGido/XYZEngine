#pragma once
#include "Camera.h"
#include "RenderPass.h"
#include "XYZ/Scene/Scene.h"
#include "XYZ/Renderer/SkeletalMesh.h"

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
		glm::vec3 ViewPosition;
	};

	class SceneRenderer
	{
	public:
		static void Init();
		static void Shutdown();

		static void SetViewportSize(uint32_t width, uint32_t height);

		static void BeginScene(const Scene* scene, const SceneRendererCamera& camera);
		static void BeginScene(const Scene* scene, const glm::mat4 viewProjectionMatrix, const glm::vec3& viewPosition);
		static void EndScene();

		static void SubmitSkeletalMesh(SkeletalMesh* mesh);
		static void SubmitSprite(SpriteRenderer* sprite, TransformComponent* transform);
		static void SubmitEditorSprite(EditorSpriteRenderer* sprite, TransformComponent* transform);
		static void SubmitCollision(TransformComponent* transform, uint32_t collisionID);
		static void SubmitParticles(ParticleComponent* particle, TransformComponent* transform);
		static void SubmitLight(PointLight2D* light, const glm::mat4& transform);
		static void SetGridProperties(const GridProperties& props);

		static void UpdateViewportSize();

		static Ref<RenderPass> GetFinalRenderPass();
		static Ref<RenderPass> GetCollisionRenderPass();

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