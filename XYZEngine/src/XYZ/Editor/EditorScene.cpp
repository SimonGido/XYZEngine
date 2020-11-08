#include "stdafx.h"
#include "EditorScene.h"

#include "XYZ/Event/InputEvent.h"
#include "XYZ/Core/Input.h"

#include "EditorEntity.h"
#include "XYZ/Renderer/Renderer.h"
#include "XYZ/Renderer/GuiRenderer.h"

#include <glm/gtx/transform.hpp>

namespace XYZ {

	static glm::vec2 MouseToWorld(const glm::vec2& point, const glm::vec2& windowSize)
	{
		glm::vec2 offset = { windowSize.x / 2,windowSize.y / 2 };
		return { point.x - offset.x, offset.y - point.y };
	}

	static bool Collide(const glm::vec2& pos, const glm::vec2& size, const glm::vec2& point)
	{
		return (pos.x + size.x / 2.0f > point.x &&
			    pos.x -	size.x / 2.0f < point.x&&
			    pos.y + size.y / 2.0f >  point.y &&
			    pos.y - size.y / 2.0f < point.y);
	}


	EditorScene::EditorScene()
	{
		Ref<FrameBuffer> fbo = FrameBuffer::Create({ 1280, 720,{0.1f,0.1f,0.1f,1.0f},1,FrameBufferFormat::RGBA16F,true });
		fbo->CreateColorAttachment(FrameBufferFormat::RGBA16F); // Position color buffer
		fbo->CreateDepthAttachment();
		fbo->Resize();
		m_RenderPass = RenderPass::Create({ fbo });

		m_ButtonGroup = m_ECS.CreateGroup<Button, CanvasRenderer, RectTransform>();
		m_SliderGroup = m_ECS.CreateGroup<Slider, CanvasRenderer, RectTransform>();

		m_TransformStorage = m_ECS.GetComponentStorage<RectTransform>();
		m_CanvasRenderStorage = m_ECS.GetComponentStorage<CanvasRenderer>();

		m_CanvasEntity = m_ECS.CreateEntity();
		EditorEntity entity(m_CanvasEntity, this);

		entity.EmplaceComponent<Canvas>(CanvasRenderMode::ScreenSpace);
		entity.EmplaceComponent<RectTransform>(glm::vec3(0, 0, 0), m_ViewportSize);
		auto renderer = entity.EmplaceComponent<CanvasRenderer>();

		int32_t transformIndex = m_ECS.GetComponentIndex<RectTransform>(m_CanvasEntity);
		int32_t rendererIndex = m_ECS.GetComponentIndex<CanvasRenderer>(m_CanvasEntity);

		m_Entities.push_back(Node{ m_CanvasEntity, transformIndex, rendererIndex });
	}
	EditorEntity EditorScene::CreateEntity(const std::string& name)
	{
		EditorEntity entity(m_ECS.CreateEntity(), this);
		entity.EmplaceComponent<RectTransform>(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec2(50.0f, 50.0f));
		entity.EmplaceComponent<CanvasRenderer>();

		int32_t transformIndex = m_ECS.GetComponentIndex<RectTransform>(entity);
		int32_t rendererIndex = m_ECS.GetComponentIndex<CanvasRenderer>(entity);
		
		m_Entities[0].Children.push_back(Node{ entity, transformIndex, rendererIndex });
		return entity;
	}
	void EditorScene::SetParent(EditorEntity parent, EditorEntity child)
	{
		auto newChildNode = findEntityNode(m_Entities.back(), parent);
		swapEntityNodes(m_Entities.back(), *newChildNode, child);
		markNodeDirty(newChildNode->Children.back());
	}
	void EditorScene::SetViewportSize(uint32_t width, uint32_t height)
	{
		m_ViewportSize = glm::vec2(width, height);

		EditorEntity canvasEntity(m_CanvasEntity, this);
		canvasEntity.GetComponent<RectTransform>()->Size = m_ViewportSize;

		auto& specs = m_RenderPass->GetSpecification().TargetFramebuffer->GetSpecification();
		specs.Width = width;
		specs.Height = height;
		m_RenderPass->GetSpecification().TargetFramebuffer->Resize();
	}

	bool EditorScene::onMouseButtonPress(MouseButtonPressEvent& event)
	{
		if (event.IsButtonPressed(MouseCode::XYZ_MOUSE_BUTTON_LEFT))
		{
			auto [mx, my] = Input::GetMousePosition();
			glm::vec2 mousePosition = MouseToWorld({ mx,my }, m_ViewportSize);
			for (int i = 0; i < m_ButtonGroup->Size(); ++i)
			{
				auto [button, canvasRenderer, rectTransform] = (*m_ButtonGroup)[i];
				if (Collide(rectTransform->WorldPosition, rectTransform->Size, mousePosition))
				{
					if (button->Execute<ClickEvent>(ClickEvent{}))
						return true;
				}
			}
		}
		return false;
	}

	bool EditorScene::onMouseButtonRelease(MouseButtonReleaseEvent& event)
	{
		if (event.IsButtonReleased(MouseCode::XYZ_MOUSE_BUTTON_LEFT))
		{
			auto [mx, my] = Input::GetMousePosition();
			glm::vec2 mousePosition = MouseToWorld({ mx,my }, m_ViewportSize);
			for (int i = 0; i < m_ButtonGroup->Size(); ++i)
			{
				auto [button, canvasRenderer, rectTransform] = (*m_ButtonGroup)[i];
				if (Collide(rectTransform->WorldPosition, rectTransform->Size, mousePosition))
				{
					if (button->Execute<ReleaseEvent>(ReleaseEvent{}))
						return true;
				}
			}
		}
		return false;
	}

	bool EditorScene::onMouseMove(MouseMovedEvent& event)
	{
		auto [mx, my] = Input::GetMousePosition();
		glm::vec2 mousePosition = MouseToWorld({ mx,my }, m_ViewportSize);
		for (int i = 0; i < m_ButtonGroup->Size(); ++i)
		{
			auto [button, canvasRenderer, rectTransform] = (*m_ButtonGroup)[i];
			if (Collide(rectTransform->WorldPosition, rectTransform->Size, mousePosition))
			{
				button->Hoovered = true;
				canvasRenderer->Color = { 0,0,1,1 };
				button->Execute<HooverEvent>(HooverEvent{});
				return true;
			}
			else if (button->Hoovered)
			{
				button->Hoovered = false;
				canvasRenderer->Color = { 1,1,1,1 };
				button->Execute<UnHooverEvent>(UnHooverEvent{});
			}
		}
		return false;
	}

	void EditorScene::markNodeDirty(Node& node)
	{
		auto& rectTransform = (*m_TransformStorage)[node.RectTransformIndex];
		rectTransform.Dirty = true;
		for (auto& it : node.Children)
			markNodeDirty(it);
	}

	void EditorScene::submitNode(const Node& node, const glm::vec3& parentPosition, bool parentDirty)
	{
		auto& rectTransform = (*m_TransformStorage)[node.RectTransformIndex];
		if (rectTransform.Dirty || parentDirty)
		{
			parentDirty = true;
			rectTransform.WorldPosition = parentPosition + rectTransform.Position;
			rectTransform.Dirty = false;
		}
		
		auto& canvasRenderer = (*m_CanvasRenderStorage)[node.CanvasRendererIndex];
		if (canvasRenderer.IsVisible)
		{
			// TODO: Temporary
			if (canvasRenderer.Material)
				GuiRenderer::SubmitWidget(&canvasRenderer, rectTransform.WorldPosition, rectTransform.Size);
			for (auto& child : node.Children)
				submitNode(child, rectTransform.WorldPosition, parentDirty);
		}
	}

	void EditorScene::swapEntityNodes(Node& current, Node& newNode, EditorEntity entity)
	{
		for (uint32_t i = 0; i < current.Children.size(); ++i)
		{
			if ((uint32_t)current.Children[i].Entity == (uint32_t)entity)
			{
				if (newNode.Entity == current.Entity)
					return;

				newNode.Children.insert(newNode.Children.end(), 
					std::make_move_iterator(current.Children.begin() + i),
					std::make_move_iterator(current.Children.begin() + i + 1)
				);
				if (i < current.Children.size() - 1)
					current.Children[i] = std::move(current.Children.back());
				current.Children.pop_back();				
				return;
			}
			swapEntityNodes(current.Children[i], newNode, entity);
		}
	}

	Node* EditorScene::findEntityNode(Node& node, EditorEntity entity)
	{
		if ((uint32_t)node.Entity == (uint32_t)entity)
		{
			return &node;
		}
		for (auto& it : node.Children)
		{
			if (auto node = findEntityNode(it, entity))
				return node;
		}
		return nullptr;
	}
	
	void EditorScene::DestroyEntity(EditorEntity entity)
	{
	}
	void EditorScene::OnPlay()
	{
	}
	void EditorScene::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		if (dispatcher.Dispatch<MouseButtonPressEvent>(Hook(&EditorScene::onMouseButtonPress, this)))
		{}
		else if (dispatcher.Dispatch<MouseButtonReleaseEvent>(Hook(&EditorScene::onMouseButtonRelease, this)))
		{}
		else if (dispatcher.Dispatch<MouseMovedEvent>(Hook(&EditorScene::onMouseMove, this)))
		{}
	}
	void EditorScene::OnUpdate(Timestep ts)
	{
		
	}
	void EditorScene::OnRender()
	{
		Renderer::BeginRenderPass(m_RenderPass, false);
		GuiRenderer::BeginScene(m_ViewportSize);
		for (auto& child : m_Entities)
			submitNode(child, glm::vec3(0.0f), false);

		GuiRenderer::EndScene();
		Renderer::EndRenderPass();
		Renderer::WaitAndRender();
	}
}