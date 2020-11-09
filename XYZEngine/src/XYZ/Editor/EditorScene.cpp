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
			pos.x - size.x / 2.0f < point.x &&
			pos.y + size.y / 2.0f > point.y &&
			pos.y - size.y / 2.0f < point.y);
	}

	static void SetMeshColor(Mesh& mesh, const glm::vec4& color)
	{
		for (auto& it : mesh.Vertices)
			it.Color = color;
	}

	static void SetQuadTexCoords(Mesh& mesh, const glm::vec4& texCoord)
	{
		glm::vec2 texCoords[4] = {
			{texCoord.x,texCoord.y},
			{texCoord.z,texCoord.y},
			{texCoord.z,texCoord.w},
			{texCoord.x,texCoord.w}
		};
		for (uint32_t i = 0; i < 4; ++i)
			mesh.Vertices[i].TexCoord = texCoords[i];
	}

	static void GenerateQuadMesh(const glm::vec4& texCoord, const glm::vec4& color, const glm::vec2& size, Mesh& mesh)
	{
		constexpr size_t quadVertexCount = 4;

		glm::vec2 texCoords[quadVertexCount] = {
			{texCoord.x,texCoord.y},
			{texCoord.z,texCoord.y},
			{texCoord.z,texCoord.w},
			{texCoord.x,texCoord.w}
		};
		glm::vec3 vertices[quadVertexCount] = {
			{  -size.x / 2.0f,  -size.y / 2.0f, 0.0f},
			{  size.x / 2.0f,  -size.y / 2.0f, 0.0f},
			{  size.x / 2.0f,  size.y / 2.0f, 0.0f},
			{  -size.x / 2.0f,  size.y / 2.0f, 0.0f}
		};
		for (size_t i = 0; i < quadVertexCount; ++i)
			mesh.Vertices.push_back(Vertex{ color, vertices[i], texCoords[i] });
	}

	static void GenerateTextMesh(const char* source, const Ref<Font>& font, const glm::vec4& color, Mesh& mesh, TextAlignment alignment)
	{
		size_t oldMeshSize = mesh.Vertices.size();
		float height = 0.0f;
		float xCursor = 0.0f;
		float yCursor = 0.0f;
		uint32_t counter = 0;
		while (source[counter] != '\0')
		{
			auto& character = font->GetCharacter(source[counter]);
			glm::vec2 charSize = {
				character.X1Coord - character.X0Coord,
				character.Y1Coord - character.Y0Coord
			};
			if (height < charSize.y) height = charSize.y;

			glm::vec2 charOffset = { character.XOffset, charSize.y - character.YOffset };
			glm::vec2 charPosition = { xCursor + charOffset.x, yCursor - charOffset.y };
			glm::vec4 charTexCoord = {
				(float)character.X0Coord / (float)font->GetWidth(), (float)character.Y0Coord / (float)font->GetHeight(),
				(float)character.X1Coord / (float)font->GetWidth(), (float)character.Y1Coord / (float)font->GetHeight()
			};

			glm::vec3 quads[4] = {
				{ charPosition.x,			   charPosition.y , 0.0f },
				{ charPosition.x + charSize.x, charPosition.y, 0.0f, },
				{ charPosition.x + charSize.x, charPosition.y + charSize.y, 0.0f, },
				{ charPosition.x,			   charPosition.y + charSize.y, 0.0f, },
			};
			glm::vec2 texCoords[4] = {
				 {charTexCoord.x, charTexCoord.w},
				 {charTexCoord.z, charTexCoord.w},
				 {charTexCoord.z, charTexCoord.y},
				 {charTexCoord.x, charTexCoord.y},
			};
			for (uint32_t i = 0; i < 4; ++i)
				mesh.Vertices.push_back(Vertex{ color, quads[i], texCoords[i] });

			xCursor += character.XAdvance;
			counter++;
		}
		if (alignment == TextAlignment::Center)
		{
			for (size_t i = oldMeshSize; i < mesh.Vertices.size(); ++i)
			{
				mesh.Vertices[i].Position.x -= xCursor / 2.0f;
				mesh.Vertices[i].Position.y -= height / 2.0f;
			}
		}
	}

	EditorScene::EditorScene(const EditorUISpecification& specs)
		:
		m_Specification(specs)
	{
		Ref<FrameBuffer> fbo = FrameBuffer::Create({ 1280, 720,{0.1f,0.1f,0.1f,1.0f},1,FrameBufferFormat::RGBA16F,true });
		fbo->CreateColorAttachment(FrameBufferFormat::RGBA16F); // Position color buffer
		fbo->CreateDepthAttachment();
		fbo->Resize();
		m_RenderPass = RenderPass::Create({ fbo });

		m_ButtonGroup = m_ECS.CreateGroup<Button, CanvasRenderer, RectTransform>();
		m_CheckboxGroup = m_ECS.CreateGroup<Checkbox, CanvasRenderer, RectTransform>();
		m_SliderGroup = m_ECS.CreateGroup<Slider, CanvasRenderer, RectTransform>();
		m_TextGroup = m_ECS.CreateGroup<Text, CanvasRenderer, RectTransform>();

		m_TransformStorage = m_ECS.GetComponentStorage<RectTransform>();
		m_CanvasRenderStorage = m_ECS.GetComponentStorage<CanvasRenderer>();
	}

	EditorEntity EditorScene::CreateCanvas(const CanvasSpecification& specs)
	{
		auto& texCoords = m_Specification.SubTexture[EditorUISpecification::BUTTON]->GetTexCoords();
		EditorEntity entity(m_ECS.CreateEntity(), this);
		Mesh mesh;
		GenerateQuadMesh(texCoords, specs.Color, glm::vec2(1.0f), mesh);
		entity.EmplaceComponent<Canvas>(specs.RenderMode);
		entity.EmplaceComponent<RectTransform>(specs.Position, specs.Size);
		auto renderer = entity.EmplaceComponent<CanvasRenderer>(
			m_Specification.Material,
			mesh,
			0,
			0,
			true
			);

		int32_t transformIndex = m_ECS.GetComponentIndex<RectTransform>(entity);
		int32_t rendererIndex = m_ECS.GetComponentIndex<CanvasRenderer>(entity);

		m_Entities.push_back(Node{ entity, transformIndex, rendererIndex });
		return entity;
	}

	EditorEntity EditorScene::CreateButton(EditorEntity canvas, const ButtonSpecification& specs)
	{
		auto& texCoords = m_Specification.SubTexture[EditorUISpecification::BUTTON]->GetTexCoords();
		EditorEntity entity(m_ECS.CreateEntity(), this);
		Mesh mesh;
		GenerateQuadMesh(texCoords, specs.DefaultColor, glm::vec2(1.0f), mesh);

		entity.EmplaceComponent<RectTransform>(specs.Position, specs.Size);
		entity.EmplaceComponent<CanvasRenderer>(
			m_Specification.Material,
			mesh,
			0,
			0,
			true
			);
		entity.EmplaceComponent<Button>(specs.DefaultColor, specs.ClickColor, specs.HooverColor);

		int32_t transformIndex = m_ECS.GetComponentIndex<RectTransform>(entity);
		int32_t rendererIndex = m_ECS.GetComponentIndex<CanvasRenderer>(entity);

		auto newChildNode = findEntityNode(m_Entities.back(), canvas);
		XYZ_ASSERT(newChildNode, "Canvas was not found");
		newChildNode->Children.push_back(Node{ entity, transformIndex, rendererIndex });
		
		EditorEntity textEntity(m_ECS.CreateEntity(), this);	
		Mesh textMesh;
		GenerateTextMesh(specs.Name.c_str(), m_Specification.Font, specs.DefaultColor, textMesh,TextAlignment::Center);
		textEntity.EmplaceComponent<RectTransform>(glm::vec3(0.0f), glm::vec2(1.0f));
		textEntity.EmplaceComponent<CanvasRenderer>(
			m_Specification.Material,
			textMesh,
			1,
			0,
			true
		);
		textEntity.EmplaceComponent<Text>(
			specs.Name,
			m_Specification.Font,
			TextAlignment::Center
		);
		int32_t textTransformIndex = m_ECS.GetComponentIndex<RectTransform>(textEntity);
		int32_t textRendererIndex = m_ECS.GetComponentIndex<CanvasRenderer>(textEntity);

		newChildNode->Children.back().Children.push_back(Node{ entity, textTransformIndex, textRendererIndex });
		return entity;
	}
	EditorEntity EditorScene::CreateCheckbox(EditorEntity canvas, const CheckboxSpecification& specs)
	{
		auto& texCoords = m_Specification.SubTexture[EditorUISpecification::CHECKBOX_UNCHECKED]->GetTexCoords();
		EditorEntity entity(m_ECS.CreateEntity(), this);
		Mesh mesh;
		GenerateQuadMesh(texCoords, specs.DefaultColor, glm::vec2(1.0f), mesh);

		entity.EmplaceComponent<RectTransform>(specs.Position, specs.Size);
		entity.EmplaceComponent<CanvasRenderer>(
			m_Specification.Material,
			mesh,
			0,
			0,
			true
			);
		entity.EmplaceComponent<Button>(specs.DefaultColor, specs.ClickColor, specs.HooverColor);

		int32_t transformIndex = m_ECS.GetComponentIndex<RectTransform>(entity);
		int32_t rendererIndex = m_ECS.GetComponentIndex<CanvasRenderer>(entity);

		auto newChildNode = findEntityNode(m_Entities.back(), canvas);
		XYZ_ASSERT(newChildNode, "Canvas was not found");
		newChildNode->Children.push_back(Node{ entity, transformIndex, rendererIndex });

		EditorEntity textEntity(m_ECS.CreateEntity(), this);
		Mesh textMesh;
		GenerateTextMesh(specs.Name.c_str(), m_Specification.Font, specs.DefaultColor, textMesh, TextAlignment::Center);
		textEntity.EmplaceComponent<RectTransform>(glm::vec3(0.0f), glm::vec2(1.0f));
		textEntity.EmplaceComponent<CanvasRenderer>(
			m_Specification.Material,
			textMesh,
			1,
			0,
			true
			);
		textEntity.EmplaceComponent<Text>(
			specs.Name,
			m_Specification.Font,
			TextAlignment::Center
			);
		int32_t textTransformIndex = m_ECS.GetComponentIndex<RectTransform>(textEntity);
		int32_t textRendererIndex = m_ECS.GetComponentIndex<CanvasRenderer>(textEntity);

		newChildNode->Children.back().Children.push_back(Node{ entity, textTransformIndex, textRendererIndex });
		return entity;
	}
	EditorEntity EditorScene::CreateText(EditorEntity canvas, const TextSpecification& specs)
	{
		EditorEntity entity(m_ECS.CreateEntity(), this);
		Mesh mesh;
		GenerateTextMesh(specs.Source.c_str(), m_Specification.Font, specs.Color, mesh, specs.Alignment);
		
		entity.EmplaceComponent<RectTransform>(specs.Position, specs.Size);
		entity.EmplaceComponent<CanvasRenderer>(
			m_Specification.Material,
			mesh,
			1,
			0,
			true
			);
		entity.EmplaceComponent<Text>(
			specs.Source,
			m_Specification.Font,
			specs.Alignment
			);

		int32_t transformIndex = m_ECS.GetComponentIndex<RectTransform>(entity);
		int32_t rendererIndex = m_ECS.GetComponentIndex<CanvasRenderer>(entity);
		
		auto newChildNode = findEntityNode(m_Entities.back(), canvas);
		XYZ_ASSERT(newChildNode, "Canvas was not found");
		newChildNode->Children.push_back(Node{ entity, transformIndex, rendererIndex });

		return entity;
	}
	void EditorScene::SetParent(EditorEntity parent, EditorEntity child)
	{
		auto newChildNode = findEntityNode(m_Entities.back(), parent);
		swapEntityNodes(m_Entities.back(), *newChildNode, child);
	}

	void EditorScene::SetViewportSize(uint32_t width, uint32_t height)
	{
		m_ViewportSize = glm::vec2(width, height);

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
				if (Collide(rectTransform->WorldPosition, rectTransform->Scale, mousePosition))
				{
					SetMeshColor(canvasRenderer->Mesh, button->ClickColor);
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
				if (Collide(rectTransform->WorldPosition, rectTransform->Scale, mousePosition))
				{
					SetMeshColor(canvasRenderer->Mesh, button->DefaultColor);
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
			if (Collide(rectTransform->WorldPosition, rectTransform->Scale, mousePosition))
			{
				SetMeshColor(canvasRenderer->Mesh, button->HooverColor);
				button->Execute<HooverEvent>(HooverEvent{});
				return true;
			}
			else
			{
				SetMeshColor(canvasRenderer->Mesh, button->DefaultColor);
				button->Execute<UnHooverEvent>(UnHooverEvent{});
			}
		}
		return false;
	}


	void EditorScene::submitNode(const Node& node, const glm::vec3& parentPosition)
	{
		auto& rectTransform = (*m_TransformStorage)[node.RectTransformIndex];
		rectTransform.WorldPosition = parentPosition + rectTransform.Position;
		auto& canvasRenderer = (*m_CanvasRenderStorage)[node.CanvasRendererIndex];
		if (canvasRenderer.IsVisible)
		{
			GuiRenderer::SubmitWidget(&canvasRenderer, &rectTransform);
			for (auto& child : node.Children)
				submitNode(child, rectTransform.WorldPosition);
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
		{
		}
		else if (dispatcher.Dispatch<MouseButtonReleaseEvent>(Hook(&EditorScene::onMouseButtonRelease, this)))
		{
		}
		else if (dispatcher.Dispatch<MouseMovedEvent>(Hook(&EditorScene::onMouseMove, this)))
		{
		}
	}
	void EditorScene::OnUpdate(Timestep ts)
	{

	}
	void EditorScene::OnRender()
	{
		Renderer::BeginRenderPass(m_RenderPass, false);
		GuiRenderer::BeginScene(m_ViewportSize);
		for (auto& child : m_Entities)
			submitNode(child, glm::vec3(0.0f));

		GuiRenderer::EndScene();
		Renderer::EndRenderPass();
		Renderer::WaitAndRender();
	}
}