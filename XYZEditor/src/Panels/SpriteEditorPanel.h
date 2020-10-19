#pragma once

#include <XYZ.h>


namespace XYZ {
	class SpriteEditorPanel
	{
	public:
		SpriteEditorPanel(AssetManager& assetManager);

		void SetContext(const Ref<Texture2D>& context);

		bool OnInGuiRender(Timestep ts);

		void OnEvent(Event& event);

		Ref<SubTexture2D> GetSelectedSprite() const;

	private:
		bool onMouseButtonRelease(MouseButtonReleaseEvent& event);
		bool onMouseButtonPress(MouseButtonPressEvent& event);
		bool onWindowResize(WindowResizeEvent& event);

		void onInGuiWindowResize(const glm::vec2& size);
		void submitSelection(const glm::vec4& selection, const glm::vec4& color);
		void onRender(Timestep ts);
		void keepCameraOnContext();
	private:
		AssetManager& m_AssetManager;
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
		bool m_SelectionOpen = false;
		bool m_ExportOpen = false;
		bool m_ActiveWindow = false;
		bool m_Selecting = false;

		int m_SelectedSelection;
		glm::vec4 m_NewSelection;
		glm::vec2 m_ContextScale;
		glm::vec2 m_ContextPos;

		std::vector<glm::vec4> m_Selections;
		std::vector<Ref<SubTexture2D>> m_Sprites;

		const glm::vec4 m_SelectionsColor = { 0.2f,0.0f,1.0f,1.0f };
		const glm::vec4 m_NewSelectionColor = { 0.0f,1.0f,1.0f,1.0f };


		static constexpr int32_t sc_InvalidSelection = -1;
	};
}