#pragma once
#include "XYZ/Renderer/Texture.h"
#include "XYZ/Renderer/SubTexture.h"
#include "XYZ/Renderer/Material.h"
#include "XYZ/Renderer/RenderPass.h"
#include "XYZ/Editor/EditorCamera.h"
#include "XYZ/Core/Timestep.h"
#include "XYZ/Scene/EditorComponents.h"
#include "XYZ/Scene/Components.h"
#include "XYZ/Event/Event.h"

namespace XYZ {
	namespace Editor {
		class SpriteEditor
		{
		public:
			SpriteEditor();

			void OnUpdate(Timestep ts);
			void OnImGuiRender(bool &open);

			void OnEvent(Event& event);

			void SetContext(const Ref<Texture2D>& context);
		private:
			void handlePanelResize(const glm::vec2& newSize);
			

			void viewer();
			void tools();
		private:
			Ref<Texture2D>		 m_Context;
			Ref<SubTexture>		 m_Output;
			EditorSpriteRenderer m_SpriteRenderer;
			TransformComponent   m_Transform;

			Ref<RenderPass> m_RenderPass;
			EditorCamera    m_Camera;
			glm::vec2	    m_ViewportSize;
			float			m_ToolSectionWidth;
			float			m_ViewSectionWidth;
			uint8_t*	    m_PixelBuffer;

			bool m_ViewportFocused;
			bool m_ViewportHovered;
		};
	}
}