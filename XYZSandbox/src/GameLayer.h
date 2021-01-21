#pragma once

#include <XYZ.h>


namespace XYZ {

	class GameLayer : public XYZ::Layer
	{
	public:
		GameLayer();
		virtual ~GameLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(Timestep ts) override;
		virtual void OnEvent(Event& event) override;
		virtual void OnInGuiRender() override;

	private:
		bool onWindowResize(WindowResizeEvent& event);

	private:
		Ref<Scene> m_Scene;

		EditorCamera m_EditorCamera;

		Entity m_Entity;

		bool m_CheckboxVal = false;
	};

}