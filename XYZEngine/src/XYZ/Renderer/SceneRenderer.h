#pragma once
#include "Camera.h"
#include "RenderPass.h"
#include "Renderer2D.h"
#include "XYZ/Scene/Scene.h"
#include "XYZ/Scene/Components.h"
#include "XYZ/Scene/EditorComponents.h"

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

	struct RenderQueue
	{
		struct SpriteDrawCommand
		{
			Ref<Material>   Material;
			Ref<SubTexture> SubTexture;
			uint32_t		SortLayer;
			glm::vec4		Color;
			glm::mat4		Transform;
		};
		struct DrawCommand
		{
			Ref<RendererCommand> Command;
			glm::mat4			 Transform;
		};

		std::vector<SpriteDrawCommand>	 m_SpriteDrawList;		
		std::vector<DrawCommand>         m_DrawCommandList;
		std::vector<Mesh>				 m_MeshCommandList;
	};

	

	class SceneRenderer : public RefCount
	{
	public:
		SceneRenderer();
		~SceneRenderer();

		virtual void Release() const override;

		void SetRenderer2D(const Ref<Renderer2D>& renderer2D);
		void SetViewportSize(uint32_t width, uint32_t height);
		void BeginScene(const Scene* scene, const SceneRendererCamera& camera);
		void BeginScene(const Scene* scene, const glm::mat4 viewProjectionMatrix, const glm::vec3& viewPosition);
		void EndScene();
		void SubmitSprite(Ref<Material> material, Ref<SubTexture> subTexture, uint32_t sortLayer, const glm::vec4& color, const glm::mat4& transform);

		void SubmitRendererCommand(Ref<RendererCommand> command, const glm::mat4& transform);
		void SubmitMesh(Ref<Mesh> mesh, const glm::mat4& transform);
		void SubmitMeshInstanced(Ref<Mesh)
		void SubmitLight(const PointLight2D& light, const glm::mat4& transform);
		void SubmitLight(const SpotLight2D& light, const glm::mat4& transform);
		void SubmitLight(const PointLight2D& light, const glm::vec3& position);
		void SubmitLight(const SpotLight2D& light, const glm::vec3& position);
		void SetGridProperties(const GridProperties& props);

		void UpdateViewportSize();

		Ref<RenderPass>		  GetFinalRenderPass();
		uint32_t			  GetFinalColorBufferRendererID();
		SceneRendererOptions& GetOptions();
	
	private:
		void flush();
		void flushLightQueue();
		void flushDefaultQueue();
		void sortQueue(RenderQueue& queue);

		void geometryPass(RenderQueue& queue, const Ref<RenderPass>& pass, bool clear);
		void lightPass();
		void bloomPass();

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
			glm::vec4 ViewPosition;
		};

		const Scene*		 m_ActiveScene;
		Ref<Renderer2D>		 m_Renderer2D;
		SceneRendererCamera  m_SceneCamera;
		SceneRendererOptions m_Options;
		GridProperties		 m_GridProps;
		glm::ivec2			 m_ViewportSize;

		// Passes
		Ref<RenderPass>		m_CompositePass;
		Ref<RenderPass>		m_LightPass;
		Ref<RenderPass>		m_GeometryPass;
		Ref<RenderPass>		m_BloomPass;
		Ref<UniformBuffer>  m_CameraUniformBuffer;

		Ref<Shader>			m_CompositeShader;
		Ref<Shader>			m_LightShader;
		Ref<Shader>			m_BloomShader;
		Ref<Texture2D>		m_BloomTexture[3];

		Ref<ShaderStorageBuffer> m_LightStorageBuffer;
		Ref<ShaderStorageBuffer> m_SpotLightStorageBuffer;

		

		enum { DefaultQueue, LightQueue, NumQueues };

		CameraData				m_CameraBuffer;
		RenderQueue				m_Queues[NumQueues];
		std::vector<PointLight>	m_PointLightsList;
		std::vector<SpotLight>	m_SpotLightsList;
		
		bool				    m_ViewportSizeChanged = false;
		int32_t					m_ThreadIndex;

		static constexpr uint32_t sc_MaxNumberOfLights = 10 * 1024;
	};
}