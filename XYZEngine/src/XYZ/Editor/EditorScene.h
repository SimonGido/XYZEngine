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

	class EditorScene
	{
	public:
		EditorScene();

		EditorEntity CreateEntity(const std::string& name);
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

		void markNodeDirty(Node& node);
		void submitNode(const Node& node, const glm::vec3& parentPosition, bool parentDirty);
		void swapEntityNodes(Node& current, Node& newNode, EditorEntity entity);
		Node* findEntityNode(Node& node,EditorEntity entity);
	private:
		Tree m_Entities;
		ECSManager m_ECS;
		uint32_t m_CanvasEntity;
		Ref<RenderPass> m_RenderPass;

		std::shared_ptr<ComponentStorage<RectTransform>> m_TransformStorage;
		std::shared_ptr<ComponentStorage<CanvasRenderer>> m_CanvasRenderStorage;

		ComponentGroup<Button, CanvasRenderer, RectTransform>* m_ButtonGroup;
		ComponentGroup<Slider, CanvasRenderer, RectTransform>* m_SliderGroup;

		glm::vec2 m_ViewportSize = glm::vec2(1280.0f, 720.0f);

		friend class EditorEntity;
	};



}