#pragma once

#include <XYZ.h>

#include "../Tools/EditorCamera.h"

namespace XYZ {
	class SpriteEditorPanel
	{
	public:
		SpriteEditorPanel();

		void SetContext(const Ref<Texture2D>& context);

		bool OnInGuiRender(Timestep ts);

		void OnEvent(Event& event);

		Ref<SubTexture2D> GetSelectedSprite() const;

	private:
		bool onMouseButtonRelease(MouseButtonReleaseEvent& event);
		bool onMouseButtonPress(MouseButtonPressEvent& event);

		void onInGuiWindowResize(const glm::vec2& size);
		void submitSelection(const glm::vec4& selection, const glm::vec4& color);
		void onRender(Timestep ts);
	private:
		enum TextureID
		{
			BACKGROUND = 0,
			CONTEXT,
		};

		Ref<Texture2D> m_Context;
		Ref<Texture2D> m_BackgroundTexture;

		Ref<FrameBuffer> m_FBO;
		Ref<Material> m_Material;

		EditorCamera m_Camera;
		glm::mat4 m_Transform;

		InGuiWindow* m_Window;
		bool m_Selecting = false;
		bool m_MenuOpen = false;
		bool m_ActiveWindow = false;

		int m_SelectedSelection;
		glm::vec4 m_NewSelection;
		glm::vec2 m_ContextScale;
		glm::vec2 m_ContextPos;

		std::vector<glm::vec4> m_Selections;
		std::vector<Ref<SubTexture2D>> m_Sprites;

		const glm::vec4 m_SelectionsColor = { 0.2f,0.0f,1.0f,1.0f };
		const glm::vec4 m_NewSelectionColor = { 0.0f,1.0f,1.0f,1.0f };
		const uint32_t m_SpriteEditorID = 6;

		static constexpr int sc_InvalidSelection = -1;
	};
}