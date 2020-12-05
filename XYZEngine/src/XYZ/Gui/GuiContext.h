#pragma once
#include "XYZ/ECS/ECSManager.h"
#include "XYZ/Event/Event.h"
#include "XYZ/Event/ApplicationEvent.h"
#include "XYZ/Core/Timestep.h"
#include "XYZ/Renderer/RenderPass.h"
#include "XYZ/Renderer/Camera.h"
#include "XYZ/Renderer/SubTexture.h"
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
#include "InputField.h"
#include "GuiSpecification.h"


namespace XYZ {
	struct TextCanvasRendererRebuild : public CanvasRendererRebuildSpecification
	{
		virtual void Rebuild(Entity entity) override;
	};

	struct QuadCanvasRendererRebuild : public CanvasRendererRebuildSpecification
	{
		virtual void Rebuild(Entity entity) override;
	};


	// TODO: same serialization logic as with scene
	class GuiContext : public RefCount
	{
	public:
		// Templates to create widgets
		Entity CreateCanvas(const CanvasSpecification& specs);
		Entity CreatePanel(uint32_t parent, const PanelSpecification& specs);
		Entity CreateButton(uint32_t parent, const ButtonSpecification& specs);
		Entity CreateCheckbox(uint32_t parent, const CheckboxSpecification& specs);
		Entity CreateSlider(uint32_t parent, const SliderSpecification& specs);
		Entity CreateText(uint32_t parent, const TextSpecification& specs);
		Entity CreateImage(uint32_t parent, const ImageSpecification& specs);
		Entity CreateInputField(uint32_t parent, const InputFieldSpecification& specs);

		void SetViewportSize(uint32_t width, uint32_t height);
		void SetParent(uint32_t parent, uint32_t child);
		void OnEvent(Event& event);
		void OnUpdate(Timestep ts);
		void OnRender();

		uint32_t GetCanvas(size_t index) const { return m_Canvases[index]; }
	private:
		// Only gui layer can create new context
		GuiContext(ECSManager* ecs, const GuiSpecification& specs);

		// Event resolving functions
		bool onCanvasRendererRebuild(CanvasRendererRebuildEvent& event);
		bool onMouseButtonPress(MouseButtonPressEvent& event);
		bool onMouseButtonRelease(MouseButtonReleaseEvent& event);
		bool onWindowResize(WindowResizeEvent& event);
		bool onMouseMove(MouseMovedEvent& event);
		bool onKeyPressed(KeyPressedEvent& event);
		bool onKeyTyped(KeyTypedEvent& event);

		// Widget specific respones
		bool buttonOnMouseButtonPress(const glm::vec2& mousePosition);
		bool buttonOnMouseButtonRelease();
		bool buttonOnMouseMove(const glm::vec2& mousePosition);

		bool checkboxOnMouseButtonPress(const glm::vec2& mousePosition);
		bool checkboxOnMouseMove(const glm::vec2& mousePosition);

		bool sliderOnMouseButtonPress(const glm::vec2& mousePosition);
		bool sliderOnMouseButtonRelease();
		bool sliderOnMouseMove(const glm::vec2& mousePosition);

		bool inputFieldOnMouseButtonPress(const glm::vec2& mousePosition);
		bool inputFieldOnMouseMove(const glm::vec2& mousePosition);

		// Update functions
		void updateTransform(uint32_t entity, const glm::vec3& parentPosition);
		void applyLayoutGroup(const LayoutGroup& layout, const Relationship& parentRel, const RectTransform& transform);

	private:
		//Tree m_Entities;
		Camera m_Camera;
		glm::mat4 m_ViewMatrix;

		ECSManager* m_ECS = nullptr;
		ComponentView<CanvasRenderer, RectTransform>* m_RenderView;
		ComponentView<Canvas, CanvasRenderer, RectTransform>* m_CanvasView;
		ComponentView<Button, CanvasRenderer, RectTransform>* m_ButtonView;
		ComponentView<Checkbox, CanvasRenderer, RectTransform>* m_CheckboxView;
		ComponentView<Slider, CanvasRenderer, RectTransform>* m_SliderView;
		ComponentView<LayoutGroup, Relationship, RectTransform>* m_LayoutGroupView;
		ComponentView<Layout, Relationship, CanvasRenderer, RectTransform>* m_LayoutView;
		ComponentView<InputField, CanvasRenderer, RectTransform>* m_InputFieldView;
		GuiSpecification m_Specification;

		Ref<RenderPass> m_RenderPass;
		glm::vec2 m_ViewportSize;

		std::vector<uint32_t> m_Canvases;

		friend class GuiLayer;
		friend class Serializer;
		friend class Dockspace;
	};
}