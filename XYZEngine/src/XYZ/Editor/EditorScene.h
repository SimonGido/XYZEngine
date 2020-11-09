#pragma once
#include "XYZ/Event/Event.h"
#include "XYZ/Core/Timestep.h"
#include "XYZ/ECS/ECSManager.h"
#include "XYZ/Renderer/RenderPass.h"

#include "XYZ/Gui/Button.h"
#include "XYZ/Gui/Checkbox.h"
#include "XYZ/Gui/Slider.h"
#include "XYZ/Gui/Text.h"

#include "XYZ/Gui/Canvas.h"
#include "XYZ/Gui/CanvasRenderer.h"
#include "XYZ/Gui/RectTransform.h"
#include "XYZ/Gui/Text.h"


#include "EditorUISpecification.h"

namespace XYZ {


	// TODO: if performance issue handle tree smarter
	class EditorEntity;
	struct Node
	{
		uint32_t Entity;
		int32_t RectTransformIndex;
		int32_t CanvasRendererIndex;

		std::vector<Node> Children;
	};
	using Tree = std::vector<Node>;

	class EditorScene : public RefCount,
						public Serializable
	{
	public:
		EditorScene(const EditorUISpecification& specs);

		//TODO: Create new class UIContext or something like that
		EditorEntity CreateCanvas(const CanvasSpecification& specs);
		EditorEntity CreateButton(EditorEntity canvas, const ButtonSpecification& specs);
		EditorEntity CreateCheckbox(EditorEntity canvas, const CheckboxSpecification& specs);
		EditorEntity CreateText(EditorEntity canvas, const TextSpecification& specs);

		void SetParent(EditorEntity parent, EditorEntity child);

		void DestroyEntity(EditorEntity entity);
		void OnPlay();
		void OnEvent(Event& e);
		void OnUpdate(Timestep ts);
		void OnRender();

		void SetViewportSize(uint32_t width, uint32_t height);


	private:
		bool onMouseButtonPress(MouseButtonPressEvent& event);
		bool onMouseButtonRelease(MouseButtonReleaseEvent& event);
		bool onMouseMove(MouseMovedEvent& event);

		void submitNode(const Node& node, const glm::vec3& parentPosition);
		void swapEntityNodes(Node& current, Node& newNode, EditorEntity entity);
		Node* findEntityNode(Node& node,EditorEntity entity);
	
	private:
		Tree m_Entities;
		ECSManager m_ECS;
		Ref<RenderPass> m_RenderPass;
		EditorUISpecification m_Specification;

		std::shared_ptr<ComponentStorage<RectTransform>> m_TransformStorage;
		std::shared_ptr<ComponentStorage<CanvasRenderer>> m_CanvasRenderStorage;

		ComponentGroup<Button, CanvasRenderer, RectTransform>* m_ButtonGroup;
		ComponentGroup<Checkbox, CanvasRenderer, RectTransform>* m_CheckboxGroup;
		ComponentGroup<Slider, CanvasRenderer, RectTransform>* m_SliderGroup;
		ComponentGroup<Text, CanvasRenderer, RectTransform>* m_TextGroup;

		glm::vec2 m_ViewportSize = glm::vec2(1280.0f, 720.0f);

		friend class EditorEntity;
	};



}