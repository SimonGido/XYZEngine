#pragma once

#include "XYZ/ECS/ECSManager.h"
#include "XYZ/ECS/Types.h"

#include "XYZ/Event/Event.h"
#include "XYZ/Event/InputEvent.h"
#include "XYZ/Event/GuiEvent.h"
#include "XYZ/Scene/Components.h"

#include "XYZ/Core/Timestep.h"
#include "GuiSpecification.h"
#include "RectTransform.h"
#include "GuiContext.h"

#include <glm/glm.hpp>


namespace XYZ {

	enum class SplitType
	{
		None,
		Horizontal,
		Vertical
	};

	struct DockNodeComponent : public ECS::Type<DockNodeComponent>
	{
		DockNodeComponent(const glm::vec3& position, const glm::vec2& size);

		std::vector<uint32_t> Entities;
		glm::vec3 Position;
		glm::vec2 Size;
		SplitType Split = SplitType::None;
	};

	class Dockspace
	{
	public:
		Dockspace(ECS::ECSManager* ecs, GuiContext* context);

		void OnUpdate(Timestep ts);
		void OnEvent(Event& event);


		uint32_t CreatePanel(uint32_t canvas, const std::string& name, const PanelSpecification& specs);

	private:
		bool onMouseButtonPress(MouseButtonPressEvent& event);
		bool onMouseButtonRelease(MouseButtonReleaseEvent& event);
		bool onButtonPress(ClickEvent& event);
		bool onButtonRelease(ReleaseEvent& event);

		void updateEntities(uint32_t nodeEntity);
		void destroyNode(uint32_t nodeEntity);
		bool removeFromNode(uint32_t nodeEntity, uint32_t entity);
		void insertToNode(uint32_t nodeEntity, uint32_t entity);
		uint32_t splitNode(uint32_t entity, SplitType type, const glm::vec2& mousePosition);
		uint32_t createNode(uint32_t parent, const glm::vec3& position, const glm::vec2& size);
		uint32_t findNode(uint32_t entity, const glm::vec2& mousePosition);
		SplitType resolveSplit(uint32_t nodeEntity, const glm::vec2& mousePosition);
	private:
		ECS::ECSManager* m_ECS;
		ECS::ComponentView<DockNodeComponent>* m_DockView;
		
		uint32_t m_RootEntity;
		
		uint32_t m_ModifiedEntity = NULL_ENTITY;
		glm::vec2 m_MouseOffset = glm::vec2(0.0f);

		GuiContext* m_Context;
	};
}