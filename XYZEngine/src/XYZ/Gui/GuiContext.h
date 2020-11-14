#pragma once
#include "XYZ/ECS/ECSManager.h"
#include "XYZ/Event/Event.h"
#include "XYZ/Event/ApplicationEvent.h"
#include "XYZ/Core/Timestep.h"
#include "XYZ/Renderer/RenderPass.h"
#include "XYZ/Renderer/Camera.h"

#include "Canvas.h"
#include "Button.h"
#include "Checkbox.h"
#include "Slider.h"
#include "Text.h"
#include "CanvasRenderer.h"
#include "RectTransform.h"
#include "GuiSpecification.h"


namespace XYZ {
	struct TextCanvasRendererRebuild : public CanvasRendererRebuildSpecification
	{
		TextCanvasRendererRebuild(Text* text);

		virtual void Rebuild(CanvasRenderer* renderer, RectTransform* transform) override;

	private:
		Text* m_Text;
	};

	struct QuadCanvasRendererRebuild : public CanvasRendererRebuildSpecification
	{
		QuadCanvasRendererRebuild(const glm::vec4& color, const glm::vec4& texCoords);
			
		virtual void Rebuild(CanvasRenderer* renderer, RectTransform* transform) override;
		
	private:
		glm::vec4 m_Color;
		glm::vec4 m_TexCoords;
	};

	struct Node
	{
		uint32_t Entity;
		std::vector<Node> Children;
	};

	using Tree = std::vector<Node>;

	class GuiContext
	{
	public:
		uint32_t CreateCanvas(const CanvasSpecification& specs);
		uint32_t CreatePanel(uint32_t canvas, const PanelSpecification& specs);
		uint32_t CreateButton(uint32_t canvas, const ButtonSpecification& specs);
		uint32_t CreateCheckbox(uint32_t canvas, const CheckboxSpecification& specs);
		//uint32_t CreateSlider(uint32_t canvas);
		uint32_t CreateText(uint32_t canvas, const TextSpecification& specs);

		void SetViewportSize(uint32_t width, uint32_t height);
		void SetParent(uint32_t parent, uint32_t child);
		void OnEvent(Event& event);
		void OnUpdate(Timestep ts);
		void OnRender();


	private:
		// Only gui layer can create new context
		GuiContext(ECS::ECSManager* ecs, const GuiSpecification& specs);

		bool onCanvasRendererRebuild(CanvasRendererRebuildEvent& event);

		bool onWindowResizeEvent(WindowResizeEvent& event);
		bool onMouseButtonPress(MouseButtonPressEvent& event);
		bool onMouseButtonRelease(MouseButtonReleaseEvent& event);
		bool onMouseMove(MouseMovedEvent& event);

		bool buttonOnMouseButtonPress(const glm::vec2& mousePosition);
		bool buttonOnMouseButtonRelease();
		bool buttonOnMouseMove(const glm::vec2& mousePosition);

		bool checkboxOnMouseButtonPress(const glm::vec2& mousePosition);
		bool checkboxOnMouseMove(const glm::vec2& mousePosition);


		void submitNode(const Node& node, const glm::vec3& parentPosition);
		void swapEntityNodes(Node& current, Node& newNode, uint32_t entity);
		Node* findEntityNode(Node& node, uint32_t entity);
	
	private:
		Tree m_Entities;
		Camera m_Camera;
		glm::mat4 m_ViewMatrix;

		ECS::ECSManager* m_ECS = nullptr;
		ECS::ComponentView<Canvas, CanvasRenderer, RectTransform>* m_CanvasView;
		ECS::ComponentView<Button, CanvasRenderer, RectTransform>* m_ButtonView;
		ECS::ComponentView<Checkbox, CanvasRenderer, RectTransform>* m_CheckboxView;

		GuiSpecification m_Specification;

		Ref<RenderPass> m_RenderPass;
		glm::vec2 m_ViewportSize;



		friend class GuiLayer;
	};

}