#pragma once
#include "XYZ/Event/Event.h"
#include "XYZ/Core/Timestep.h"
#include "XYZ/ECS/ECSManager.h"
#include "XYZ/Renderer/RenderPass.h"

#include "Canvas.h"
#include "Button.h"
#include "Checkbox.h"
#include "Slider.h"
#include "Text.h"
#include "CanvasRenderer.h"
#include "RectTransform.h"
#include "GuiSpecification.h"


namespace XYZ {
	struct Node
	{
		uint32_t Entity;
		int32_t RectTransformIndex;
		int32_t CanvasRendererIndex;

		std::vector<Node> Children;
	};
	using Tree = std::vector<Node>;

	class GuiContext
	{
	public:
		uint32_t CreateCanvas(const CanvasSpecification& specs);
		uint32_t CreateButton(uint32_t canvas, const ButtonSpecification& specs);
		uint32_t CreateCheckbox(uint32_t canvas, const CheckboxSpecification& specs);
		uint32_t CreateText(uint32_t canvas, const TextSpecification& specs);
		
		void SetViewportSize(uint32_t width, uint32_t height);
		void SetParent(uint32_t parent, uint32_t child);
		void OnEvent(Event& event);
		void OnUpdate(Timestep ts);
		void OnRender();

	private:
		GuiContext(ECSManager* ecs, const GuiSpecification& specs);


		bool onMouseButtonPress(MouseButtonPressEvent& event);
		bool onMouseButtonRelease(MouseButtonReleaseEvent& event);
		bool onMouseMove(MouseMovedEvent& event);

		void submitNode(const Node& node, const glm::vec3& parentPosition);
		void swapEntityNodes(Node& current, Node& newNode, uint32_t entity);
		Node* findEntityNode(Node& node, uint32_t entity);
	
	private:
		Tree m_Entities;
		ECSManager* m_ECS = nullptr;
		GuiSpecification m_Specification;
		Ref<RenderPass> m_RenderPass;
		glm::vec2 m_ViewportSize;

		std::shared_ptr<ComponentStorage<RectTransform>> m_TransformStorage;
		std::shared_ptr<ComponentStorage<CanvasRenderer>> m_CanvasRenderStorage;

		ComponentGroup<Button, CanvasRenderer, RectTransform>* m_ButtonGroup;
		ComponentGroup<Checkbox, CanvasRenderer, RectTransform>* m_CheckboxGroup;
		ComponentGroup<Slider, CanvasRenderer, RectTransform>* m_SliderGroup;
		ComponentGroup<Text, CanvasRenderer, RectTransform>* m_TextGroup;

		friend class GuiLayer;
	};

}