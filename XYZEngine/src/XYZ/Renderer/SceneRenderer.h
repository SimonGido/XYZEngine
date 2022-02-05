#pragma once
#include "Camera.h"
#include "RenderPass.h"
#include "Renderer2D.h"
#include "Mesh.h"
#include "RenderCommandBuffer.h"
#include "StorageBufferSet.h"

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

	struct SpriteRenderData
	{
		Ref<SubTexture> SubTexture;
		glm::vec4		Color;
		glm::mat4		Transform;
	};

	struct RenderQueue
	{
		struct SpriteDrawData
		{
			Ref<Material>	Material;
			Ref<SubTexture>	SubTexture;
			glm::vec4		Color;
			glm::mat4		Transform;
		};

		// TODO: this is not really a draw command
		struct BillboardDrawCommand
		{
			Ref<Material>   Material;
			Ref<SubTexture> SubTexture;
			uint32_t		SortLayer;
			glm::vec4		Color;
			glm::vec3		Position;
			glm::vec2	    Size;
		};

		struct DrawMeshCommand
		{
			Ref<Mesh> Mesh;
			glm::mat4 Transform;
		};
		struct InstancedDrawMeshCommand
		{
			Ref<Mesh> Mesh;
			glm::mat4 Transform;
			uint32_t  Count;
		};

		std::vector<SpriteDrawData>			  m_SpriteDrawList;		
		std::vector<BillboardDrawCommand>	  m_BillboardDrawList;
		std::vector<DrawMeshCommand>	      m_MeshCommandList;
		std::vector<InstancedDrawMeshCommand> m_InstancedMeshCommandList;
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
		void SubmitMesh(const Ref<Mesh>& mesh, const glm::mat4& transform);
		void SubmitMeshInstanced(const Ref<Mesh>& mesh, const glm::mat4& transform, uint32_t count);
		void SubmitMeshInstanced(const Ref<Mesh>& mesh, const std::vector<glm::mat4>& transforms, uint32_t count);



		void OnImGuiRender();

		Ref<Renderer2D>	      GetRenderer2D() const { return m_Renderer2D; }
		Ref<RenderPass>		  GetFinalRenderPass() const;
		Ref<Image2D>		  GetFinalPassImage() const;
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

		void createCompositePipeline();
		void createLightPipeline();
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

		SceneRendererSpecification m_Specification;
		Ref<Scene>				   m_ActiveScene;
		Ref<Renderer2D>			   m_Renderer2D;
		
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
		
		// Passes
		Ref<RenderPass>			   m_GeometryPass;
		Ref<RenderPass>			   m_BloomPass;
		Ref<UniformBuffer>		   m_CameraUniformBuffer;
								   
		Ref<Shader>				   m_BloomComputeShader;
		Ref<Texture2D>			   m_BloomTexture[3];


		


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


			static constexpr uint32_t Count() { return sizeof(GPUTimeQueries) / sizeof(uint32_t); }
		};
		GPUTimeQueries m_GPUTimeQueries;
	};
}