#pragma once

#include <XYZ.h>


namespace XYZ {
	class SpriteEditorPanel
	{
	public:
		SpriteEditorPanel();

		void SetContext(const Ref<Texture2D>& context);

		bool OnInGuiRender();
		void OnEvent(Event& event);

		Ref<SubTexture2D> GetSelectedSprite() const;

	private:
		bool onMouseButtonRelease(MouseButtonReleaseEvent& event);
		bool onMouseButtonPress(MouseButtonPressEvent& event);

		void onInGuiWindowResize(const glm::vec2& size);

	private:
		Ref<Texture2D> m_Context;
		Ref<Texture2D> m_BackgroundTexture;

		InGuiWindow* m_Window;
		bool m_Selecting = false;
		bool m_MenuOpen = false;

		float m_ZoomLevel;
		int m_SelectedSelection;
		glm::vec2 m_ContextSize;
		glm::vec4 m_NewSelection;

		std::vector<glm::vec4> m_Selections;
		std::vector<Ref<SubTexture2D>> m_Sprites;

		const glm::vec4 m_SelectionsColor = { 0.2f,0.0f,1.0f,1.0f };
		const uint32_t m_SpriteEditorID = 6;

		static constexpr int sc_InvalidSelection = -1;
	};
}