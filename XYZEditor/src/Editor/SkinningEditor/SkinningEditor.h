#pragma once
#include "Editor/EditorPanel.h"
#include "Editor/EditorCamera.h"

#include "Editor/OrthographicCameraController.h"

#include "XYZ/Renderer/StorageBufferSet.h"

namespace XYZ {
	namespace Editor {
		class SkinningEditor : public EditorPanel
		{
		public:
			SkinningEditor(std::string name);

			virtual void OnImGuiRender(bool& open) override;
			virtual void OnUpdate(Timestep ts) override;
			virtual void SetSceneContext(const Ref<Scene>& scene) override;
			virtual bool OnEvent(Event& e) override;

			void SetContext(const Ref<SubTexture>& subTexture);
		private:
			bool handleToolbar();
			void handleStateActions();
			void createPipeline();

			glm::vec2 getMouseWorldPosition() const;
		private:
			Ref<Scene>				 m_Scene;
			Ref<RenderCommandBuffer> m_CommandBuffer;
			Ref<Renderer2D>			 m_Renderer2D;

			Ref<SubTexture>    m_Context;
			Ref<Texture2D>	   m_Texture;
			Ref<AnimatedMesh>  m_Mesh;
			Ref<MaterialAsset> m_MaterialAsset;
			Ref<Pipeline>	   m_Pipeline;
			Ref<StorageBufferSet> m_BoneTransformsStorageSet;
			Ref<VertexBuffer>	  m_TransformsVertexBuffer;

			OrthographicCameraController m_CameraController;

			enum class State
			{
				None,
				PlacingPoints
			};

			State m_State = State::None;

			struct GPUTimeQueries
			{
				uint32_t GPUTime = 0;

				static constexpr uint32_t Count() { return sizeof(GPUTimeQueries) / sizeof(uint32_t); }
			};
			GPUTimeQueries m_GPUTimeQueries;

			glm::vec2 m_ViewportSize{};
			glm::vec2 m_MousePosition{};
			glm::vec2 m_ButtonSize;
			float m_ContextAspectRatio = 1.0f;

			bool m_UpdateViewport = false;
			bool m_Hoovered = false;
			bool m_Focused = false;
			
			struct Triangle
			{
				uint32_t First, Second, Third;
			};

			std::vector<glm::vec2> m_Points;
			std::vector<Triangle>  m_Triangles;


			enum Color
			{
				Points,
				Selected,
				Lines,
				NumColors
			};

			glm::vec4 m_Colors[NumColors];
		};
	}
}