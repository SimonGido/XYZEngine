#pragma once

#include "XYZ/Event/Event.h"
#include "XYZ/Event/InputEvent.h"
#include "XYZ/Event/GuiEvent.h"
#include "XYZ/Scene/Components.h"

#include "XYZ/Core/Timestep.h"
#include "GuiSpecification.h"
#include "RectTransform.h"
#include "DockNode.h"
#include "GuiContext.h"



namespace XYZ {

	class Dockspace
	{
	public:
		Dockspace();
		Dockspace(GuiContext* context, uint32_t root);
		Dockspace(const Dockspace& other);
		~Dockspace();

		Dockspace& operator=(const Dockspace& other);

		void OnUpdate(Timestep ts);
		void OnEvent(Event& event);

		Entity CreatePanel(uint32_t canvas, const std::string& name, const PanelSpecification& specs);
		Entity CreateRenderWindow(uint32_t canvas, const std::string& name, const ImageSpecification& specs);

	private:
		bool onWindowResize(WindowResizeEvent& event);
		bool onMouseButtonPress(MouseButtonPressEvent& event);
		bool onMouseButtonRelease(MouseButtonReleaseEvent& event);
		bool onButtonPress(ClickEvent& event);
		bool onButtonRelease(ReleaseEvent& event);


		void handleResize();
		void updateEntities(uint32_t nodeEntity);
		void destroyNode(uint32_t nodeEntity);
		void adjustNodeChildren(uint32_t nodeEntity);
		void adjustEntityTransform(uint32_t nodeEntity, uint32_t entity);
		void insertToNode(uint32_t nodeEntity, uint32_t entity);
		bool removeFromNode(uint32_t nodeEntity, uint32_t entity);
		void resizeNode(uint32_t nodeEntity, const glm::vec3& positionDiff, const glm::vec2& sizeDiff);
		void rescaleNode(uint32_t nodeEntity, const glm::vec2 scale);
		uint32_t splitNode(uint32_t nodeEntity, SplitType type, const glm::vec2& mousePosition);
		uint32_t createNode(uint32_t parent, const glm::vec3& position, const glm::vec2& size);
		uint32_t findNode(uint32_t entity, const glm::vec2& mousePosition);
		uint32_t findResize(uint32_t nodeEntity, const glm::vec2& mousePosition);
		SplitType resolveSplit(uint32_t nodeEntity, const glm::vec2& mousePosition);
	
	private:
		GuiContext* m_Context;
		uint32_t m_RootEntity = NULL_ENTITY;
		uint32_t m_ModifiedEntity = NULL_ENTITY;

		glm::vec2 m_MouseOffset = glm::vec2(0.0f);

		struct ResizeData
		{
			uint32_t  Entity = NULL_ENTITY;
			SplitType Type = SplitType::None;
		};

		ResizeData m_ResizeData;
		
		std::vector<size_t> m_CallbackIDs;

		friend class Serializer; 
	};
}