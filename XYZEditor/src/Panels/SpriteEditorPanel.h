#pragma once

#include <XYZ.h>

#include "Panel.h"

namespace XYZ {
	class SpriteEditorPanel : public Panel,
							  public EventCaller
	{
	public:
		SpriteEditorPanel(uint32_t id);

		virtual void OnInGuiRender() override;
		virtual void OnUpdate(Timestep ts) override;
		virtual void OnEvent(Event& event) override;

		Ref<SubTexture2D> GetSelectedSprite() const;

		void SetContext(const Ref<Texture2D>& context);
	private:
		bool onMouseButtonRelease(MouseButtonReleaseEvent& event);
		bool onMouseButtonPress(MouseButtonPressEvent& event);
		bool onWindowResize(WindowResizeEvent& event);

		void onInGuiWindowResize(const glm::vec2& size);
		void submitSelection(const glm::vec4& selection, const glm::vec4& color);
		void onRender(Timestep ts);
		void keepCameraOnContext();
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

		bool m_SelectionOpen = false;
		bool m_ExportOpen = false;
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