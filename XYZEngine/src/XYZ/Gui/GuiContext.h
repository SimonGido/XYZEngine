#pragma once
#include "XYZ/ECS/ECSManager.h"
#include "XYZ/Event/Event.h"
#include "XYZ/Event/ApplicationEvent.h"
#include "XYZ/Core/Timestep.h"
#include "XYZ/Renderer/RenderPass.h"
#include "XYZ/Renderer/Camera.h"
#include "XYZ/Renderer/SubTexture2D.h"
#include "XYZ/Scene/Components.h"
#include "XYZ/Core/Ref.h"

#include "Canvas.h"
#include "Button.h"
#include "Checkbox.h"
#include "Slider.h"
#include "Text.h"
#include "CanvasRenderer.h"
#include "RectTransform.h"
#include "LayoutGroup.h"
#include "GuiSpecification.h"


namespace XYZ {
	struct TextCanvasRendererRebuild : public CanvasRendererRebuildSpecification
	{
		virtual void Rebuild(uint32_t entity, ECS::ECSManager& ecs) override;
	};

	struct QuadCanvasRendererRebuild : public CanvasRendererRebuildSpecification
	{
		virtual void Rebuild(uint32_t entity, ECS::ECSManager& ecs) override;
	};

	struct Node
	{
		uint32_t Entity;
		std::vector<Node> Children;
	};

	using Tree = std::vector<Node>;

	class GuiContext : public RefCount
	{
	public:
		// Templates to create widgets
		uint32_t CreateCanvas(const CanvasSpecification& specs);
		uint32_t CreatePanel(uint32_t parent, const PanelSpecification& specs);
		uint32_t CreateButton(uint32_t parent, const ButtonSpecification& specs);
		uint32_t CreateCheckbox(uint32_t parent, const CheckboxSpecification& specs);
		uint32_t CreateText(uint32_t parent, const TextSpecification& specs);
		uint32_t CreateImage(uint32_t parent, const ImageSpecification& specs);
	
		//uint32_t CreateSlider(uint32_t canvas);

		void SetViewportSize(uint32_t width, uint32_t height);
		void SetParent(uint32_t parent, uint32_t child);
		void OnEvent(Event& event);
		void OnUpdate(Timestep ts);
		void OnRender();

		uint32_t GetCanvas(size_t index) const { return m_Canvases[index]; }
	private:
		// Only gui layer can create new context
		GuiContext(ECS::ECSManager* ecs, const GuiSpecification& specs);

		// Event resolving functions
		bool onCanvasRendererRebuild(CanvasRendererRebuildEvent& event);
		bool onWindowResizeEvent(WindowResizeEvent& event);
		bool onMouseButtonPress(MouseButtonPressEvent& event);
		bool onMouseButtonRelease(MouseButtonReleaseEvent& event);
		bool onMouseMove(MouseMovedEvent& event);

		// Widget specific respones
		bool buttonOnMouseButtonPress(const glm::vec2& mousePosition);
		bool buttonOnMouseButtonRelease();
		bool buttonOnMouseMove(const glm::vec2& mousePosition);
		bool checkboxOnMouseButtonPress(const glm::vec2& mousePosition);
		bool checkboxOnMouseMove(const glm::vec2& mousePosition);

		// Update functions
		void submitToRenderer(uint32_t entity, const glm::vec3& parentPosition);
		void applyLayout(const LayoutGroup& layout, const Relationship& parentRel, const RectTransform& transform);
	private:
		//Tree m_Entities;
		Camera m_Camera;
		glm::mat4 m_ViewMatrix;

		ECS::ECSManager* m_ECS = nullptr;
		ECS::ComponentView<Canvas, CanvasRenderer, RectTransform>* m_CanvasView;
		ECS::ComponentView<Button, CanvasRenderer, RectTransform>* m_ButtonView;
		ECS::ComponentView<Checkbox, CanvasRenderer, RectTransform>* m_CheckboxView;
		ECS::ComponentView<LayoutGroup, Relationship, RectTransform>* m_LayoutGroup;

		GuiSpecification m_Specification;

		Ref<RenderPass> m_RenderPass;
		glm::vec2 m_ViewportSize;

		std::vector<uint32_t> m_Canvases;

		friend class GuiLayer;
	};

}