#pragma once
#include "Editor/EditorPanel.h"
#include "Editor/EditorCamera.h"

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


		private:
			Ref<Scene>				 m_Scene;
			Ref<RenderCommandBuffer> m_CommandBuffer;
			Ref<Renderer2D>			 m_Renderer2D;


			Ref<SubTexture>    m_Context;
			Ref<Texture2D>	   m_Texture;
			Ref<AnimatedMesh>  m_Mesh;
			Ref<MaterialAsset> m_MaterialAsset;


			EditorCamera m_Camera;

			struct GPUTimeQueries
			{
				uint32_t GPUTime = 0;

				static constexpr uint32_t Count() { return sizeof(GPUTimeQueries) / sizeof(uint32_t); }
			};
			GPUTimeQueries m_GPUTimeQueries;


			bool m_Hoovered = false;
			bool m_Focused = false;
			glm::vec2 m_ViewportSize{};
			bool m_UpdateViewport = false;

			std::vector<glm::vec2> m_Points;
		};
	}
}