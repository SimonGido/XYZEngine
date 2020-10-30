#pragma once

#include "Panel.h"
#include "../Inspectable/InspectableEntity.h"

namespace XYZ {
	class ScenePanel : public Panel, 
					   public EventCaller
	{
	public:
		ScenePanel(uint32_t id);

		virtual void OnInGuiRender() override;
		virtual void OnUpdate(Timestep ts) override;
		virtual void OnEvent(Event& event) override;
		void SetContext(const Ref<Scene>& context);

	private:
		bool onWindowResized(WindowResizeEvent& event);
		bool onMouseButtonPress(MouseButtonPressEvent& event);
		bool onMouseButtonRelease(MouseButtonReleaseEvent& event);
		bool onKeyPress(KeyPressedEvent& event);
		bool onKeyRelease(KeyReleasedEvent& event);

		void onInGuiWindowResize(const glm::vec2& size);
	
		Entity selectEntity(const glm::vec2& position);
		void updateModifiedEntity();
	private:
		Ref<Scene> m_Context;
		EditorCamera m_EditorCamera;
		bool m_Selecting = false;

		struct ModifiedEntity
		{
			XYZ::Entity Entity;
			TransformComponent* Transform;
			TransformComponent  OldTransform;
			glm::vec2	StartMousePosition = glm::vec2(0, 0);
			bool Moving = false, Scaling = false, Rotating = false;
		};
		ModifiedEntity m_ModifiedEntity;

		HashGrid2D<Entity> m_Entities;
	};
}