#pragma once

#include "Panel.h"
#include "../Inspectable/InspectableEntity.h"

namespace XYZ {

	class SceneHierarchyPanel : public Panel,
		public EventCaller
	{
	public:
		SceneHierarchyPanel(uint32_t id);
		virtual void OnInGuiRender() override;
		virtual void OnUpdate(Timestep ts) override;
		virtual void OnEvent(Event& event) override;
		void SetContext(const Ref<Scene>& context);

	private:
		bool onMouseButtonPress(MouseButtonPressEvent& event);
		bool onMouseButtonRelease(MouseButtonReleaseEvent& event);
		bool onKeyPress(KeyPressedEvent& event);
		bool onKeyRelease(KeyReleasedEvent& event);

		void drawEntity(Entity entity);
	private:
		Ref<Scene> m_Context;

		glm::vec2 m_PopupPosition = { 0,0 };
		bool m_PopupEnabled = false;
	};
}