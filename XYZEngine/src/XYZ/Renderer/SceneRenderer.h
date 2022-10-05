#pragma once
#include "Camera.h"
#include "RenderPass.h"
#include "Renderer2D.h"
#include "Mesh.h"
#include "RenderCommandBuffer.h"
#include "StorageBufferSet.h"
#include "VertexBufferSet.h"
#include "PipelineCompute.h"
#include "MaterialInstance.h"
#include "GeometryRenderQueue.h"

#include "RenderPasses/GeometryPass.h"
#include "RenderPasses/DeferredLightPass.h"
#include "RenderPasses/BloomPass.h"
#include "RenderPasses/CompositePass.h"
#include "RenderPasses/LightCullingPass.h"

#include "XYZ/Asset/Renderer/MaterialAsset.h"

#include "XYZ/Scene/Scene.h"
#include "XYZ/Scene/Components.h"


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
		glm::mat4 ViewMatrix{};
		glm::vec3 ViewPosition{};
	};

	struct SceneRendererSpecification
	{
		bool SwapChainTarget = false;
	};

	class SceneRenderer : public RefCount
	{
	public:
		SceneRenderer(Ref<Scene> scene, SceneRendererSpecification specification = {});
		~SceneRenderer();

		void Init();
		void SetScene(Ref<Scene> scene);
		void SetViewportSize(uint32_t width, uint32_t height);
		void SetGridProperties(const GridProperties& props);

		void BeginScene(const SceneRendererCamera& camera);
		void BeginScene(const glm::mat4& viewProjectionMatrix, const glm::mat4& viewMatrix, const glm::vec3& viewPosition);
		void EndScene();

		void SubmitBillboard(const Ref<MaterialAsset>& material, const Ref<SubTexture>& subTexture, uint32_t sortLayer, const glm::vec4& color, const glm::vec3& position, const glm::vec2& size);
		void SubmitSprite(const Ref<MaterialAsset>& material, const Ref<SubTexture>& subTexture, const glm::vec4& color, const glm::mat4& transform);

		void SubmitMesh(const Ref<Mesh>& mesh, const Ref<MaterialAsset>& material, const glm::mat4& transform, const Ref<MaterialInstance>& overrideMaterial = nullptr);
		void SubmitMesh(const Ref<Mesh>& mesh, const Ref<MaterialAsset>& material, const void* instanceData, uint32_t instanceCount, uint32_t instanceSize, const Ref<MaterialInstance>& overrideMaterial);
		void SubmitMesh(const Ref<AnimatedMesh>& mesh, const Ref<MaterialAsset>& material, const glm::mat4& transform, const std::vector<ozz::math::Float4x4>& boneTransforms, const Ref<MaterialInstance>& overrideMaterial = nullptr);

		void OnImGuiRender();


		Ref<RenderPass>			 GetFinalRenderPass()	  const;
		Ref<Image2D>			 GetFinalPassImage()	  const;
		Ref<RenderCommandBuffer> GetRenderCommandBuffer() const { return m_CommandBuffer; }
		Ref<UniformBufferSet>    GetUniformBufferSet()	  const { return m_UniformBufferSet; }
		
		SceneRendererOptions& GetOptions();
	private:
		void createCompositePass();
		void createLightPass();
		void createGeometryPass();
		void createDepthPass();
		void createBloomTextures();
		void createGridResources();

		void updateViewportSize();
		void preRender();
		void renderGrid();

		void updateUniformBufferSet();
	private:
		struct UBCameraData
		{
			glm::mat4 ViewProjectionMatrix;
			glm::mat4 ViewMatrix;
			glm::vec4 ViewPosition;
		} m_CameraDataUB;

		struct UBPointLights3D
		{
			uint32_t	 Count{ 0 };
			glm::vec3	 Padding{};
			PointLight3D PointLights[1024]{};
		} m_PointsLights3DUB;

		struct UBRendererData
		{
			uint32_t TilesCountX{ 0 };
			bool ShowLightComplexity = false;
			char Padding1[3] = { 0, 0, 0 };  // Bools are 4-bytes in GLSL

		} m_RendererDataUB;
		
		GeometryPass	  m_GeometryPass;
		DeferredLightPass m_DeferredLightPass;
		LightCullingPass  m_LightCullingPass;
		BloomPass		  m_BloomPass;
		CompositePass	  m_CompositePass;

		Ref<RenderPass>				  m_GeometryRenderPass;
		Ref<RenderPass>				  m_CompositeRenderPass;
		Ref<RenderPass>				  m_LightRenderPass;
		Ref<RenderPass>				  m_DepthRenderPass;

		Ref<Material>				  m_GridMaterial;
		Ref<Pipeline>				  m_GridPipeline;
		Ref<MaterialInstance>		  m_GridMaterialInstance;

		std::array<Ref<Texture2D>, 3> m_BloomTexture;

		SceneRendererSpecification m_Specification;
		Ref<Scene>				   m_ActiveScene;

		Ref<RenderCommandBuffer>   m_CommandBuffer;
		
		Ref<UniformBufferSet>      m_UniformBufferSet;
		Ref<StorageBufferSet>	   m_StorageBufferSet;

		SceneRendererCamera		   m_SceneCamera;
		SceneRendererOptions	   m_Options;
		BloomSettings			   m_BloomSettings;
		GridProperties			   m_GridProps;
		glm::ivec2				   m_ViewportSize;
		glm::ivec3				   m_LightCullingWorkGroups;

		GeometryRenderQueue		   m_Queue;								   
		bool				       m_ViewportSizeChanged = false;
	
		
		struct RenderStatistics
		{
			uint32_t SpriteDrawCommandCount = 0;
			uint32_t MeshDrawCommandCount = 0;
			uint32_t MeshOverrideDrawCommandCount = 0;
			uint32_t InstanceMeshDrawCommandCount = 0;
			uint32_t PointLight2DCount = 0;
			uint32_t SpotLight2DCount = 0;
			uint32_t TransformInstanceCount = 0;
			uint32_t InstanceDataSize = 0;
		};
		RenderStatistics m_RenderStatistics;

		struct GPUTimeQueries
		{
			uint32_t GPUTime = 0;
			uint32_t DepthPassQuery = 0;
			uint32_t GeometryPassQuery = 0;
		
			static constexpr uint32_t Count() { return sizeof(GPUTimeQueries) / sizeof(uint32_t); }
		};
		GPUTimeQueries m_GPUTimeQueries;
	};
}