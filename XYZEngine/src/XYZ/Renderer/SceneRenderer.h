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
		struct SpriteDrawCommand
		{
			Ref<Material>  Material;
			std::array<Ref<Texture2D>, Renderer2D::GetMaxTextures()> Textures;

			uint32_t       TextureCount = 0;
			uint32_t       SpriteCount = 0;

			uint32_t setTexture(const Ref<Texture2D>& texture);		
		};

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
	

		struct SpriteDrawKey
		{
			SpriteDrawKey(const AssetHandle& matHandle)
				: MaterialHandle(matHandle)
			{}

			bool operator<(const SpriteDrawKey& other) const
			{
				return (MaterialHandle < other.MaterialHandle);
			}

			AssetHandle MaterialHandle;
		};


		std::map<SpriteDrawKey, SpriteDrawCommand> SpriteDrawCommands;
		std::map<SpriteDrawKey, SpriteDrawCommand> BillboardDrawCommands;

		std::vector<SpriteDrawData>				   SpriteData;
		std::vector<BillboardDrawData>			   BillboardData;
			
	};

	struct SceneRendererSpecification
	{
		bool SwapChainTarget = false;
	};

	using RenderOverlayFn = Delegate<void(Ref<RenderCommandBuffer>, uint32_t& timeQuery)>;
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

		void OnImGuiRender();

		template <auto Callable>
		void RenderOverlay();

		template <auto Callable, typename Type>
		void RenderOverlay(Type* instance);

		void StopRenderOverlay() { m_RenderOverlayFn = {}; }

		Ref<Renderer2D>	      GetRenderer2D() const { return m_Renderer2D; }
		Ref<RenderPass>		  GetRenderer2DPass() const { return m_Renderer2D->GetTargetRenderPass(); }
		Ref<RenderPass>		  GetFinalRenderPass() const;
		Ref<Image2D>		  GetFinalPassImage() const;
		Ref<RenderCommandBuffer> GetRenderCommandBuffer() const { return m_CommandBuffer; }

		SceneRendererOptions& GetOptions();
	private:
		void flush();
		void flushLightQueue();
		void flushDefaultQueue();


		void geometryPass2D(RenderQueue& queue, bool clear);
		void lightPass();
		void bloomPass();

		void createCompositePass();
		void createLightPass();

		void updateViewportSize();
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
			
			void Init(const Ref<RenderPass>& renderPass, const Ref<Shader>& shader, const BufferLayout& layout, PrimitiveTopology topology = PrimitiveTopology::Triangles);
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
		Ref<RenderPass>			   m_CompositePass;
		Ref<RenderPass>			   m_LightPass;

		Ref<StorageBufferSet>      m_LightStorageBufferSet;

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


		struct GPUTimeQueries
		{
			uint32_t GPUTime = 0;
			uint32_t Renderer2DPassQuery = 0;
			uint32_t OverlayQuery = 0;

			static constexpr uint32_t Count() { return sizeof(GPUTimeQueries) / sizeof(uint32_t); }
		};
		GPUTimeQueries m_GPUTimeQueries;
	
		RenderOverlayFn m_RenderOverlayFn;
	};
	template<auto Callable>
	inline void SceneRenderer::RenderOverlay()
	{
		m_RenderOverlayFn.Connect<Callable>();
	}

	template<auto Callable, typename Type>
	inline void SceneRenderer::RenderOverlay(Type* instance)
	{
		m_RenderOverlayFn.Connect<Callable>(instance);
	}
}