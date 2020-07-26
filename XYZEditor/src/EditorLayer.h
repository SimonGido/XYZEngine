#pragma once

#include <XYZ.h>


namespace XYZ {

	class EditorLayer : public Layer
	{
	public:
		virtual ~EditorLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(float ts) override;
		virtual void OnEvent(Event& event) override;
		virtual void OnInGuiRender() override;

	private:
		EditorCamera m_EditorCamera;
		Ref<Scene> m_Scene;
		Ref<Material> m_Material;
		Ref<Font> m_Font;
		Ref<Material> m_TextMaterial;
		Ref<Texture2D> m_GuiTexture;
		Ref<Texture2D> m_CharacterTexture;
		Ref<SubTexture2D> m_CharacterSubTexture;
		Ref<SubTexture2D> m_CheckboxSubTexture;


		Ref<FrameBuffer> m_FBO;

	private:
		Entity m_TestEntity;
		Entity m_TextEntity;

		Transform2D* m_Transform;

		glm::vec4 m_Color = { 0,0,0,0 };
		glm::vec4 m_Pallete = { 0,1,0,1 };
		float m_TestValue = 0.0f;

		bool m_ActiveWindow = false;
	};
}