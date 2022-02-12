#pragma once
#include "Camera.h"
#include "RenderPass.h"
#include "Renderer2D.h"
#include "Mesh.h"
#include "RenderCommandBuffer.h"
#include "StorageBufferSet.h"
#include "PipelineCompute.h"

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


	struct RenderQueue
	{
		struct SpriteDrawData
		{
			uint32_t  TextureIndex;
			glm::vec4 TexCoords;
			glm::vec4 Color;
			glm::mat4 Transform;
		};

		struct BillboardDrawData
		{
			uint32_t  TextureIndex;
			glm::vec4 TexCoords;
			glm::vec4 Color;
			glm::vec3 Position;
			glm::vec2 Size;
		};

		struct SpriteDrawCommand
		{
			Ref<Material>  Material;
			std::array<Ref<Texture2D>, Renderer2D::GetMaxTextures()> Textures;

			uint32_t       TextureCount = 0;

			uint32_t setTexture(const Ref<Texture2D>& texture);

			std::vector<SpriteDrawData>		SpriteData;
			std::vector<BillboardDrawData>	BillboardData;
		};

		struct TransformData
		{
			glm::vec4 TransformRow[3];
		};

		struct MeshDrawCommand
		{
			Ref<Mesh>			   Mesh;
			Ref<Material>		   Material;
			Ref<Pipeline>		   Pipeline;
			uint32_t			   InstanceCount = 0;

			std::vector<TransformData> TransformData;
			uint32_t				   TransformOffset = 0;
		};
	
		struct InstanceMeshDrawCommand : public MeshDrawCommand
		{
			std::vector<std::byte> InstanceData;
			uint32_t			   InstanceOffset = 0;
		};


		struct SpriteKey
		{
			SpriteKey(const AssetHandle& matHandle)
				: MaterialHandle(matHandle)
			{}

			bool operator<(const SpriteKey& other) const
			{
				return (MaterialHandle < other.MaterialHandle);
			}

			AssetHandle MaterialHandle;
		};
		struct MeshKey
		{
			AssetHandle MeshHandle;
			AssetHandle MaterialHandle;

			MeshKey(AssetHandle meshHandle, AssetHandle materialHandle)
				: MeshHandle(meshHandle), MaterialHandle(materialHandle) {}

			bool operator<(const MeshKey& other) const
			{
				if (MeshHandle < other.MeshHandle)
					return true;

				return (MeshHandle == other.MeshHandle) && (MaterialHandle < other.MaterialHandle);
			}
		};

		struct InstanceMeshKey : public MeshKey
		{
			InstanceMeshKey(AssetHandle meshHandle, AssetHandle materialHandle, uint32_t instanceSize)
				: MeshKey(meshHandle, materialHandle), InstanceSize(instanceSize) {}

			bool operator<(const InstanceMeshKey& other) const
			{
				if (MeshHandle < other.MeshHandle)
					return true;

				if (MeshHandle == other.MeshHandle && InstanceSize == other.InstanceSize)
					return true;

				return (MeshHandle == other.MeshHandle) && (InstanceSize == other.InstanceSize) && (MaterialHandle < other.MaterialHandle);

			}

			uint32_t InstanceSize;
		};


		std::map<SpriteKey, SpriteDrawCommand> SpriteDrawCommands;
		std::map<SpriteKey, SpriteDrawCommand> BillboardDrawCommands;			

		std::map<MeshKey,		  MeshDrawCommand>			MeshDrawCommands;
		std::map<InstanceMeshKey, InstanceMeshDrawCommand>	InstanceMeshDrawCommands;
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

		void SubmitBillboard(const Ref<Material>& material, const Ref<SubTexture>& subTexture, uint32_t sortLayer, const glm::vec4& color, const glm::vec3& position, const glm::vec2& size);
		void SubmitSprite(const Ref<Material>& material, const Ref<SubTexture>& subTexture, const glm::vec4& color, const glm::mat4& transform);

		void SubmitMesh(const Ref<Mesh>& mesh, const Ref<Material>& material, const glm::mat4& transform);
		void SubmitMesh(const Ref<Mesh>& mesh, const Ref<Material>& material, const glm::mat4& transform, const void* instanceData, uint32_t instanceCount, uint32_t instanceSize);

		void OnImGuiRender();


		Ref<Renderer2D>	      GetRenderer2D() const { return m_Renderer2D; }
		Ref<RenderPass>		  GetRenderer2DPass() const { return m_Renderer2D->GetTargetRenderPass(); }
		Ref<RenderPass>		  GetFinalRenderPass() const;
		Ref<Image2D>		  GetFinalPassImage() const;
		Ref<RenderCommandBuffer> GetRenderCommandBuffer() const { return m_CommandBuffer; }

		SceneRendererOptions& GetOptions();
	private:
		void geometryPass(RenderQueue& queue, bool clear);
		void geometryPass2D(RenderQueue& queue, bool clear);
		void lightPass();
		void bloomPass();
		void compositePass();

		void createCompositePass();
		void createLightPass();
		void createGeometryPass();

		void updateViewportSize();

		void preRender();
		void prepareInstances();
		void prepareLights();
		
		Ref<Pipeline> createGeometryPipeline(const Ref<Material>& material, bool instanced = false);
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

		private:
			float Alignment[2];
		};
	
		struct CameraData
		{
			glm::mat4 ViewProjectionMatrix;
			glm::mat4 ViewMatrix;
			glm::vec4 ViewPosition;
		};

		struct SceneRenderPipeline
		{
			Ref<Pipeline> Pipeline;
			Ref<Material> Material;
			
			void Init(const Ref<RenderPass>& renderPass, const Ref<Shader>& shader, PrimitiveTopology topology = PrimitiveTopology::Triangles);
		};

		struct BloomSettings
		{
			float FilterTreshold = 1.0f;
			float FilterKnee = 0.1f;
		};
		BloomSettings			   m_BloomSettings;

		SceneRendererSpecification m_Specification;
		Ref<Scene>				   m_ActiveScene;
		Ref<Renderer2D>			   m_Renderer2D;
		Ref<Texture2D>			   m_WhiteTexture;

		SceneRenderPipeline		   m_CompositeRenderPipeline;
		SceneRenderPipeline		   m_LightRenderPipeline;
		
		std::map<size_t, Ref<Pipeline>> m_GeometryPipelines;

		Ref<RenderPass>			   m_CompositePass;
		Ref<RenderPass>			   m_LightPass;
		Ref<RenderPass>			   m_GeometryPass;

		Ref<StorageBufferSet>      m_LightStorageBufferSet;

		Ref<VertexBuffer>		   m_InstanceVertexBuffer;
		Ref<VertexBuffer>		   m_TransformVertexBuffer;

		std::vector<RenderQueue::TransformData> m_TransformData;
		std::vector<std::byte>	   m_InstanceData;
		std::vector<PointLight>	   m_PointLights;
		std::vector<SpotLight>	   m_SpotLights;

		SceneRendererCamera		   m_SceneCamera;
		SceneRendererOptions	   m_Options;
		GridProperties			   m_GridProps;
		glm::ivec2				   m_ViewportSize;

		Ref<RenderCommandBuffer>   m_CommandBuffer;
		

		Ref<UniformBuffer>		   m_CameraUniformBuffer;
								   

		Ref<Material>			   m_BloomComputeMaterial;
		Ref<Texture2D>			   m_BloomTexture[3];
		Ref<PipelineCompute>	   m_BloomComputePipeline;

		
		CameraData				   m_CameraBuffer;
		RenderQueue				   m_Queue;

		uint32_t				   m_NumPointLights;
		uint32_t				   m_NumSpotLights;
								   
		bool				       m_ViewportSizeChanged = false;
		int32_t					   m_ThreadIndex;

		static constexpr uint32_t sc_MaxNumberOfLights = 1024;
		static constexpr uint32_t sc_InstanceVertexBufferSize = 1 * 1024 * 1024; // 1mb
		static constexpr uint32_t sc_TransformBufferCount = 100 * 1024; // 10240 transforms

		struct GPUTimeQueries
		{
			uint32_t GPUTime = 0;
			uint32_t Renderer2DPassQuery = 0;

			static constexpr uint32_t Count() { return sizeof(GPUTimeQueries) / sizeof(uint32_t); }
		};
		GPUTimeQueries m_GPUTimeQueries;


	private:
		Ref<Mesh> m_TestMesh;
		Ref<Material> m_TestMaterial;
	};
}