#include "stdafx.h"
#include "GuiContext.h"

#include "XYZ/Event/InputEvent.h"
#include "XYZ/Core/Input.h"
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

	static void GenerateTextMesh(const char* source, const Ref<Font>& font, const glm::vec4& color, const glm::vec2& size, Mesh& mesh, TextAlignment alignment)
	{
		size_t oldMeshSize = mesh.Vertices.size();
		float height = 0.0f;
		float xCursor = 0.0f;
		float yCursor = 0.0f;
		uint32_t counter = 0;
		while (source[counter] != '\0')
		{
			auto& character = font->GetCharacter(source[counter]);
			if (xCursor + character.XAdvance >= size.x)
				break;

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




	GuiContext::GuiContext(ECSManager* ecs, const GuiSpecification& specs)
		:
		m_ECS(ecs),
		m_Specification(specs)
	{
		m_ViewMatrix = glm::inverse(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f)));
		Ref<FrameBuffer> fbo = FrameBuffer::Create({ 1280, 720,{0.1f,0.1f,0.1f,1.0f},1,FrameBufferFormat::RGBA16F,true });
		fbo->CreateColorAttachment(FrameBufferFormat::RGBA16F); // Position color buffer
		fbo->CreateDepthAttachment();
		fbo->Resize();
		m_RenderPass = RenderPass::Create({ fbo });

		m_CanvasGroup = m_ECS->CreateGroup<Canvas, CanvasRenderer, RectTransform>();
		m_ButtonGroup = m_ECS->CreateGroup<Button, CanvasRenderer, RectTransform>();
		m_CheckboxGroup = m_ECS->CreateGroup<Checkbox, CanvasRenderer, RectTransform>();
		m_SliderGroup = m_ECS->CreateGroup<Slider, CanvasRenderer, RectTransform>();
		m_TextGroup = m_ECS->CreateGroup<Text, CanvasRenderer, RectTransform>();

		m_TransformStorage = m_ECS->GetComponentStorage<RectTransform>();
		m_CanvasRenderStorage = m_ECS->GetComponentStorage<CanvasRenderer>();
	}
	bool GuiContext::onCanvasRendererRebuild(CanvasRendererRebuildEvent& event)
	{
		event.GetRenderer()->Mesh.Vertices.clear();
		event.GetSpecification().Rebuild(event.GetRenderer(), event.GetTransform());
		return true;
	}
	uint32_t GuiContext::CreateCanvas(const CanvasSpecification& specs)
	{
		auto& texCoords = m_Specification.SubTexture[GuiSpecification::BUTTON]->GetTexCoords();
		uint32_t entity = m_ECS->CreateEntity();
		
		Mesh mesh;
		GenerateQuadMesh(texCoords, specs.Color, specs.Size, mesh);
		m_ECS->EmplaceComponent<Canvas>(entity, specs.RenderMode);
		m_ECS->EmplaceComponent<RectTransform>(entity, specs.Position, specs.Size);
		
		m_ECS->EmplaceComponent<CanvasRenderer>( entity,
			m_Specification.Material,
			mesh,
			0,
			0,
			true
			);

		int32_t transformIndex = m_ECS->GetComponentIndex<RectTransform>(entity);
		int32_t rendererIndex = m_ECS->GetComponentIndex<CanvasRenderer>(entity);

		m_Entities.push_back(Node{ entity, transformIndex, rendererIndex });
		return entity;
	}
	uint32_t GuiContext::CreatePanel(uint32_t canvas, const PanelSpecification& specs)
	{
		return uint32_t();
	}
	uint32_t GuiContext::CreateButton(uint32_t canvas, const ButtonSpecification& specs)
	{
		auto& texCoords = m_Specification.SubTexture[GuiSpecification::BUTTON]->GetTexCoords();
		uint32_t entity = m_ECS->CreateEntity();
		Mesh mesh;
		GenerateQuadMesh(texCoords, specs.DefaultColor, specs.Size, mesh);

		m_ECS->EmplaceComponent<RectTransform>(entity, specs.Position, specs.Size);
		m_ECS->EmplaceComponent<CanvasRenderer>( entity,
			m_Specification.Material,
			mesh,
			0,
			0,
			true
			);
		m_ECS->EmplaceComponent<Button>(entity, specs.DefaultColor, specs.ClickColor, specs.HooverColor);

		int32_t transformIndex = m_ECS->GetComponentIndex<RectTransform>(entity);
		int32_t rendererIndex =  m_ECS->GetComponentIndex<CanvasRenderer>(entity);

		auto newChildNode = findEntityNode(m_Entities.back(), canvas);
		XYZ_ASSERT(newChildNode, "Canvas was not found");
		newChildNode->Children.push_back(Node{ entity, transformIndex, rendererIndex });

		uint32_t textEntity = m_ECS->CreateEntity();
		Mesh textMesh;
		GenerateTextMesh(specs.Name.c_str(), m_Specification.Font, specs.DefaultColor, specs.Size, textMesh, TextAlignment::Center);
		auto textRectTransform = m_ECS->EmplaceComponent<RectTransform>(textEntity, glm::vec3(0.0f), specs.Size);
		textRectTransform->RegisterCallback<CanvasRendererRebuildEvent>(Hook(&GuiContext::onCanvasRendererRebuild, this));

		m_ECS->EmplaceComponent<CanvasRenderer>(textEntity,
			m_Specification.Material,
			textMesh,
			1,
			0,
			true
		);

		m_ECS->EmplaceComponent<Text>(textEntity,
			specs.Name,
			m_Specification.Font,
			specs.DefaultColor,
			TextAlignment::Center
		);
		int32_t textTransformIndex = m_ECS->GetComponentIndex<RectTransform>(textEntity);
		int32_t textRendererIndex =  m_ECS->GetComponentIndex<CanvasRenderer>(textEntity);

		newChildNode->Children.back().Children.push_back(Node{ entity, textTransformIndex, textRendererIndex });
		return entity;
	}
	uint32_t GuiContext::CreateCheckbox(uint32_t canvas, const CheckboxSpecification& specs)
	{
		uint32_t entity = m_ECS->CreateEntity();
		auto checkbox = m_ECS->EmplaceComponent<Checkbox>(entity, specs.DefaultColor, specs.HooverColor);


		glm::vec4 texCoords = m_Specification.SubTexture[GuiSpecification::CHECKBOX_UNCHECKED]->GetTexCoords();
		if (checkbox->Checked)
			texCoords = m_Specification.SubTexture[GuiSpecification::CHECKBOX_CHECKED]->GetTexCoords();	

		Mesh mesh;
		GenerateQuadMesh(texCoords, specs.DefaultColor, specs.Size, mesh);

		m_ECS->EmplaceComponent<RectTransform>(entity, specs.Position, specs.Size);
		m_ECS->EmplaceComponent<CanvasRenderer>(entity,
			m_Specification.Material,
			mesh,
			0,
			0,
			true
		);
	
		int32_t transformIndex = m_ECS->GetComponentIndex<RectTransform>(entity);
		int32_t rendererIndex = m_ECS->GetComponentIndex<CanvasRenderer>(entity);

		auto newChildNode = findEntityNode(m_Entities.back(), canvas);
		XYZ_ASSERT(newChildNode, "Canvas was not found");
		newChildNode->Children.push_back(Node{ entity, transformIndex, rendererIndex });

		uint32_t textEntity = m_ECS->CreateEntity();
		Mesh textMesh;
		GenerateTextMesh(specs.Name.c_str(), m_Specification.Font, specs.DefaultColor,specs.Size, textMesh, TextAlignment::Center);
		auto textRectTransform = m_ECS->EmplaceComponent<RectTransform>(textEntity, glm::vec3(0.0f), glm::vec2(1.0f));
		textRectTransform->RegisterCallback<CanvasRendererRebuildEvent>(Hook(&GuiContext::onCanvasRendererRebuild, this));
		m_ECS->EmplaceComponent<CanvasRenderer>(textEntity,
			m_Specification.Material,
			textMesh,
			1,
			0,
			true
			);

		m_ECS->EmplaceComponent<Text>(textEntity,
			specs.Name,
			m_Specification.Font,
			specs.DefaultColor,
			TextAlignment::Center
			);
		int32_t textTransformIndex = m_ECS->GetComponentIndex<RectTransform>(textEntity);
		int32_t textRendererIndex = m_ECS->GetComponentIndex<CanvasRenderer>(textEntity);

		newChildNode->Children.back().Children.push_back(Node{ entity, textTransformIndex, textRendererIndex });
		return entity;
	}
	uint32_t GuiContext::CreateText(uint32_t canvas, const TextSpecification& specs)
	{
		uint32_t entity = m_ECS->CreateEntity();
		Mesh mesh;
		GenerateTextMesh(specs.Source.c_str(), m_Specification.Font, specs.Color, specs.Size, mesh, specs.Alignment);

		auto textRectTransform = m_ECS->EmplaceComponent<RectTransform>(entity, specs.Position, specs.Size);
		textRectTransform->RegisterCallback<CanvasRendererRebuildEvent>(Hook(&GuiContext::onCanvasRendererRebuild, this));
		m_ECS->EmplaceComponent<CanvasRenderer>(entity,
			m_Specification.Material,
			mesh,
			1,
			0,
			true
			);
		m_ECS->EmplaceComponent<Text>( entity,
			specs.Source,
			m_Specification.Font,
			specs.Color,
			specs.Alignment
			);

		int32_t transformIndex = m_ECS->GetComponentIndex<RectTransform>(entity);
		int32_t rendererIndex =  m_ECS->GetComponentIndex<CanvasRenderer>(entity);

		auto newChildNode = findEntityNode(m_Entities.back(), canvas);
		XYZ_ASSERT(newChildNode, "Canvas was not found");
		newChildNode->Children.push_back(Node{ entity, transformIndex, rendererIndex });

		return entity;
	}
	void GuiContext::SetViewportSize(uint32_t width, uint32_t height)
	{
		m_ViewportSize = glm::vec2(width, height);

		auto& specs = m_RenderPass->GetSpecification().TargetFramebuffer->GetSpecification();
		specs.Width = width;
		specs.Height = height;
		m_RenderPass->GetSpecification().TargetFramebuffer->Resize();

		float w = (float)width;
		float h = (float)height;
		m_Camera.SetProjectionMatrix(glm::ortho(-w * 0.5f, w * 0.5f, -h * 0.5f, h * 0.5f));

		m_ViewportSize = glm::vec2(w, h);
	}
	void GuiContext::SetParent(uint32_t parent, uint32_t child)
	{
		auto newChildNode = findEntityNode(m_Entities.back(), parent);
		swapEntityNodes(m_Entities.back(), *newChildNode, child);
	}
	void GuiContext::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);

		if (dispatcher.Dispatch<MouseButtonPressEvent>(Hook(&GuiContext::onMouseButtonPress, this)))
		{
		}
		else if (dispatcher.Dispatch<MouseButtonReleaseEvent>(Hook(&GuiContext::onMouseButtonRelease, this)))
		{
		}
		else if (dispatcher.Dispatch<MouseMovedEvent>(Hook(&GuiContext::onMouseMove, this)))
		{
		}
		else if (dispatcher.Dispatch<WindowResizeEvent>(Hook(&GuiContext::onWindowResizeEvent, this)));
		{
		}
	}
	void GuiContext::OnUpdate(Timestep ts)
	{
		for (int i = 0; i < m_CheckboxGroup->Size(); ++i)
		{
			auto [checkbox, canvasRenderer, rectTransform] = (*m_CheckboxGroup)[i];
			if (checkbox->Checked)
				checkbox->Execute<CheckedEvent>(CheckedEvent{});
		}
	}
	void GuiContext::OnRender()
	{
		Renderer::BeginRenderPass(m_RenderPass, false);
		GuiRendererCamera renderCamera;
		renderCamera.Camera = m_Camera;
		renderCamera.ViewMatrix = m_ViewMatrix;

		GuiRenderer::BeginScene(renderCamera);
		for (auto& child : m_Entities)
			submitNode(child, glm::vec3(0.0f));

		GuiRenderer::EndScene();
		Renderer::EndRenderPass();
		Renderer::WaitAndRender();
	}
	bool GuiContext::onWindowResizeEvent(WindowResizeEvent& event)
	{
		SetViewportSize((uint32_t)event.GetWidth(), (uint32_t)event.GetHeight());
		return false;
	}
	bool GuiContext::onMouseButtonPress(MouseButtonPressEvent& event)
	{
		auto [mx, my] = Input::GetMousePosition();
		glm::vec2 mousePosition = MouseToWorld({ mx,my }, m_ViewportSize);
		if (event.IsButtonPressed(MouseCode::XYZ_MOUSE_BUTTON_LEFT))
		{	
			if (buttonOnMouseButtonPress(mousePosition))
				return true;
			else if (checkboxOnMouseButtonPress(mousePosition))
				return true;
		}
		return false;
	}
	bool GuiContext::onMouseButtonRelease(MouseButtonReleaseEvent& event)
	{
		if (event.IsButtonReleased(MouseCode::XYZ_MOUSE_BUTTON_LEFT))
		{
			if (buttonOnMouseButtonRelease())
				return true;
		}
		return false;
	}
	bool GuiContext::onMouseMove(MouseMovedEvent& event)
	{
		auto [mx, my] = Input::GetMousePosition();
		glm::vec2 mousePosition = MouseToWorld({ mx,my }, m_ViewportSize);
		
		if (buttonOnMouseMove(mousePosition))
			return true;
		else if (checkboxOnMouseMove(mousePosition))
			return true;

		return false;
	}

	bool GuiContext::buttonOnMouseButtonPress(const glm::vec2& mousePosition)
	{
		for (int i = 0; i < m_ButtonGroup->Size(); ++i)
		{
			auto [button, canvasRenderer, rectTransform] = (*m_ButtonGroup)[i];
			if (Collide(rectTransform->WorldPosition, rectTransform->Size, mousePosition))
			{
				button->Machine.TransitionTo(ButtonState::Clicked);
				SetMeshColor(canvasRenderer->Mesh, button->ClickColor);
				if (button->Execute<ClickEvent>(ClickEvent{}))
					return true;
			}
		}
		return false;
	}

	bool GuiContext::buttonOnMouseButtonRelease()
	{
		for (int i = 0; i < m_ButtonGroup->Size(); ++i)
		{
			auto [button, canvasRenderer, rectTransform] = (*m_ButtonGroup)[i];
			button->Machine.TransitionTo(ButtonState::Released);
			SetMeshColor(canvasRenderer->Mesh, button->DefaultColor);
			if (button->Execute<ReleaseEvent>(ReleaseEvent{}))
				return true;
		}
		return false;
	}

	bool GuiContext::buttonOnMouseMove(const glm::vec2& mousePosition)
	{
		for (int i = 0; i < m_ButtonGroup->Size(); ++i)
		{
			auto [button, canvasRenderer, rectTransform] = (*m_ButtonGroup)[i];
			if (Collide(rectTransform->WorldPosition, rectTransform->Size, mousePosition)
				&& button->Machine.TransitionTo(ButtonState::Hoovered))
			{
				SetMeshColor(canvasRenderer->Mesh, button->HooverColor);
				button->Execute<HooverEvent>(HooverEvent{});
				return true;
			}
			else if (button->Machine.TransitionTo(ButtonState::UnHoovered))
			{
				SetMeshColor(canvasRenderer->Mesh, button->DefaultColor);
				button->Execute<UnHooverEvent>(UnHooverEvent{});
			}
		}
		return false;
	}

	bool GuiContext::checkboxOnMouseButtonPress(const glm::vec2& mousePosition)
	{
		for (int i = 0; i < m_CheckboxGroup->Size(); ++i)
		{
			auto [checkbox, canvasRenderer, rectTransform] = (*m_CheckboxGroup)[i];
			if (Collide(rectTransform->WorldPosition, rectTransform->Size, mousePosition))
			{
				if (checkbox->Checked)
				{
					checkbox->Checked = false;
					SetQuadTexCoords(canvasRenderer->Mesh, m_Specification.SubTexture[GuiSpecification::CHECKBOX_UNCHECKED]->GetTexCoords());
					if (checkbox->Execute<UnCheckedEvent>(UnCheckedEvent{}));
						return true;
				}
				else
				{
					checkbox->Checked = true;
					SetQuadTexCoords(canvasRenderer->Mesh, m_Specification.SubTexture[GuiSpecification::CHECKBOX_CHECKED]->GetTexCoords());
					if (checkbox->Execute<CheckedEvent>(CheckedEvent{}));
						return true;
				}
			}
		}
		return false;
	}

	bool GuiContext::checkboxOnMouseMove(const glm::vec2& mousePosition)
	{
		for (int i = 0; i < m_CheckboxGroup->Size(); ++i)
		{
			auto [checkbox, canvasRenderer, rectTransform] = (*m_CheckboxGroup)[i];
			if (Collide(rectTransform->WorldPosition, rectTransform->Size, mousePosition))
			{
				SetMeshColor(canvasRenderer->Mesh, checkbox->HooverColor);
				checkbox->Execute<HooverEvent>(HooverEvent{});
				return true;
			}
			else
			{
				SetMeshColor(canvasRenderer->Mesh, checkbox->DefaultColor);
				checkbox->Execute<UnHooverEvent>(UnHooverEvent{});
			}
		}
		return false;
	}

	void GuiContext::submitNode(const Node& node, const glm::vec3& parentPosition)
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
	void GuiContext::swapEntityNodes(Node& current, Node& newNode, uint32_t entity)
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
	Node* GuiContext::findEntityNode(Node& node, uint32_t entity)
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
	TextCanvasRendererRebuild::TextCanvasRendererRebuild(Text* text)
		: m_Text(text)
	{
	}
	void TextCanvasRendererRebuild::Rebuild(CanvasRenderer* renderer, RectTransform* transform)
	{
		size_t oldMeshSize = renderer->Mesh.Vertices.size();
		float height = 0.0f;
		float xCursor = 0.0f;
		float yCursor = 0.0f;
		uint32_t counter = 0;
		for (auto c : m_Text->Source)
		{
			auto& character = m_Text->Font->GetCharacter(c);
			if (xCursor + character.XAdvance >= transform->Size.x)
				break;

			glm::vec2 charSize = {
				character.X1Coord - character.X0Coord,
				character.Y1Coord - character.Y0Coord
			};
			if (height < charSize.y) height = charSize.y;

			glm::vec2 charOffset = { character.XOffset, charSize.y - character.YOffset };
			glm::vec2 charPosition = { xCursor + charOffset.x, yCursor - charOffset.y };
			glm::vec4 charTexCoord = {
				(float)character.X0Coord / (float)m_Text->Font->GetWidth(), (float)character.Y0Coord / (float)m_Text->Font->GetHeight(),
				(float)character.X1Coord / (float)m_Text->Font->GetWidth(), (float)character.Y1Coord / (float)m_Text->Font->GetHeight()
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
				renderer->Mesh.Vertices.push_back(Vertex{ m_Text->Color, quads[i], texCoords[i] });

			xCursor += character.XAdvance;
			counter++;
		}
		if (m_Text->Alignment == TextAlignment::Center)
		{
			for (size_t i = oldMeshSize; i < renderer->Mesh.Vertices.size(); ++i)
			{
				renderer->Mesh.Vertices[i].Position.x -= xCursor / 2.0f;
				renderer->Mesh.Vertices[i].Position.y -= height / 2.0f;
			}
		}
	}
	QuadCanvasRendererRebuild::QuadCanvasRendererRebuild(const glm::vec4& color, const glm::vec4& texCoords)
		:
		m_Color(color),
		m_TexCoords(texCoords)
	{
	}
	void QuadCanvasRendererRebuild::Rebuild(CanvasRenderer* renderer, RectTransform* transform)
	{
		constexpr size_t quadVertexCount = 4;
	
		glm::vec2 texCoords[quadVertexCount] = {
			{m_TexCoords.x,m_TexCoords.y},
			{m_TexCoords.z,m_TexCoords.y},
			{m_TexCoords.z,m_TexCoords.w},
			{m_TexCoords.x,m_TexCoords.w}
		};
		glm::vec3 vertices[quadVertexCount] = {
			{  -transform->Size.x / 2.0f,  -transform->Size.y / 2.0f, 0.0f},
			{   transform->Size.x / 2.0f,  -transform->Size.y / 2.0f, 0.0f},
			{   transform->Size.x / 2.0f,   transform->Size.y / 2.0f, 0.0f},
			{  -transform->Size.x / 2.0f,   transform->Size.y / 2.0f, 0.0f}
		};
		for (size_t i = 0; i < quadVertexCount; ++i)
			renderer->Mesh.Vertices.push_back(Vertex{ m_Color, vertices[i], texCoords[i] });
	}
}