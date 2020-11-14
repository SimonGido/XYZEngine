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




	GuiContext::GuiContext(ECS::ECSManager* ecs, const GuiSpecification& specs)
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

		m_CanvasView = &m_ECS->CreateView<Canvas, CanvasRenderer, RectTransform>();
		m_ButtonView = &m_ECS->CreateView<Button, CanvasRenderer, RectTransform>();
		m_CheckboxView = &m_ECS->CreateView<Checkbox, CanvasRenderer, RectTransform>();
		m_LayoutGroup = &m_ECS->CreateView<LayoutGroup, Relationship, RectTransform>();
	}
	bool GuiContext::onCanvasRendererRebuild(CanvasRendererRebuildEvent& event)
	{
		auto& renderer = m_ECS->GetComponent<CanvasRenderer>(event.GetEntity());
		renderer.Mesh.Vertices.clear();

		event.GetSpecification().Rebuild(event.GetEntity(), *m_ECS);
		return true;
	}

	uint32_t GuiContext::CreateCanvas(const CanvasSpecification& specs)
	{
		auto& texCoords = m_Specification.SubTexture[GuiSpecification::BUTTON]->GetTexCoords();
		uint32_t entity = m_ECS->CreateEntity();
		
		Mesh mesh;
		GenerateQuadMesh(texCoords, specs.Color, specs.Size, mesh);
	
		m_ECS->AddComponent<Relationship>(entity, Relationship());
		m_ECS->AddComponent<Canvas>(entity, Canvas(specs.RenderMode, specs.Color));
		m_ECS->AddComponent<RectTransform>(entity, RectTransform(specs.Position, specs.Size));
		m_ECS->AddComponent<CanvasRenderer>( entity,
			CanvasRenderer(
				m_Specification.Material,
				m_Specification.SubTexture[GuiSpecification::BUTTON],
				specs.Color,
				mesh,
				0,
				true
			));


		m_Canvases.push_back(entity);
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

		m_ECS->AddComponent<Relationship>(entity, Relationship());
		m_ECS->AddComponent<RectTransform>(entity, RectTransform( specs.Position, specs.Size));
		m_ECS->AddComponent<CanvasRenderer>( entity,
			CanvasRenderer(
				m_Specification.Material,
				m_Specification.SubTexture[GuiSpecification::BUTTON],
				specs.DefaultColor,
				mesh,
				0,
				true
		));

		m_ECS->AddComponent<Button>(entity, Button(specs.ClickColor, specs.HooverColor));
			
		setupParent(canvas, entity);
		
		uint32_t textEntity = m_ECS->CreateEntity();
		Mesh textMesh;
		GenerateTextMesh(specs.Name.c_str(), m_Specification.Font, specs.DefaultColor, specs.Size, textMesh, TextAlignment::Center);
		auto& textRectTransform = m_ECS->AddComponent<RectTransform>(textEntity, RectTransform(glm::vec3(0.0f), specs.Size));
		
		textRectTransform.RegisterCallback<CanvasRendererRebuildEvent>(Hook(&GuiContext::onCanvasRendererRebuild, this));
		
		m_ECS->AddComponent<Relationship>(textEntity, Relationship());

		m_ECS->AddComponent<CanvasRenderer>(textEntity,
			CanvasRenderer(
				m_Specification.Material,
				m_Specification.SubTexture[GuiSpecification::FONT],
				specs.DefaultColor,
				textMesh,
				0,
				true
		));
		
		m_ECS->AddComponent<Text>(textEntity,
			Text(
			specs.Name,
			m_Specification.Font,
			specs.DefaultColor,
			TextAlignment::Center
		));
		
		setupParent(entity, textEntity);
		return entity;
	}
	uint32_t GuiContext::CreateCheckbox(uint32_t canvas, const CheckboxSpecification& specs)
	{
		uint32_t entity = m_ECS->CreateEntity();
		auto& checkbox = m_ECS->AddComponent<Checkbox>(entity, Checkbox(specs.HooverColor));


		glm::vec4 texCoords = m_Specification.SubTexture[GuiSpecification::CHECKBOX_UNCHECKED]->GetTexCoords();
		if (checkbox.Checked)
			texCoords = m_Specification.SubTexture[GuiSpecification::CHECKBOX_CHECKED]->GetTexCoords();

		Mesh mesh;
		GenerateQuadMesh(texCoords, specs.DefaultColor, specs.Size, mesh);
		
		m_ECS->AddComponent<Relationship>(entity, Relationship());
		m_ECS->AddComponent<RectTransform>(entity, RectTransform(specs.Position, specs.Size));
	
		m_ECS->AddComponent<CanvasRenderer>(entity,
			CanvasRenderer(
				m_Specification.Material,
				m_Specification.SubTexture[GuiSpecification::CHECKBOX_UNCHECKED],
				specs.DefaultColor,
				mesh,
				0,
				true
			));


		setupParent(canvas, entity);

		uint32_t textEntity = m_ECS->CreateEntity();
		Mesh textMesh;
		GenerateTextMesh(specs.Name.c_str(), m_Specification.Font, specs.DefaultColor, specs.Size, textMesh, TextAlignment::Center);
		auto& textRectTransform = m_ECS->AddComponent<RectTransform>(textEntity, RectTransform(glm::vec3(0.0f), glm::vec2(1.0f)));
		textRectTransform.RegisterCallback<CanvasRendererRebuildEvent>(Hook(&GuiContext::onCanvasRendererRebuild, this));
		m_ECS->AddComponent<Relationship>(textEntity, Relationship());
		
		m_ECS->AddComponent<CanvasRenderer>(textEntity,
			CanvasRenderer(
				m_Specification.Material,
				m_Specification.SubTexture[GuiSpecification::FONT],
				specs.DefaultColor,
				textMesh,
				0,
				true
			));

		m_ECS->AddComponent<Text>(textEntity,
			Text(
				specs.Name,
				m_Specification.Font,
				specs.DefaultColor,
				TextAlignment::Center
			));
		
		setupParent(entity, textEntity);
		return entity;
	}
	uint32_t GuiContext::CreateText(uint32_t canvas, const TextSpecification& specs)
	{
		uint32_t entity = m_ECS->CreateEntity();
		Mesh mesh;
		GenerateTextMesh(specs.Source.c_str(), m_Specification.Font, specs.Color, specs.Size, mesh, specs.Alignment);

		auto& textRectTransform = m_ECS->AddComponent<RectTransform>(entity, RectTransform(specs.Position, specs.Size));
		textRectTransform.RegisterCallback<CanvasRendererRebuildEvent>(Hook(&GuiContext::onCanvasRendererRebuild, this));
		m_ECS->AddComponent<CanvasRenderer>(entity,
			CanvasRenderer(
				m_Specification.Material,
				m_Specification.SubTexture[GuiSpecification::FONT],
				specs.Color,
				mesh,
				0,
				true
			));

		m_ECS->AddComponent<Relationship>(entity, Relationship());
		m_ECS->AddComponent<Text>( entity,
			Text(
				specs.Source,
				m_Specification.Font,
				specs.Color,
				specs.Alignment
			));

		
		setupParent(canvas, entity);

		return entity;
	}
	uint32_t GuiContext::CreateImage(uint32_t canvas, const Ref<SubTexture2D>& subTexture)
	{
		auto& texCoords = subTexture->GetTexCoords();
		uint32_t entity = m_ECS->CreateEntity();

		Mesh mesh;
		GenerateQuadMesh(texCoords, glm::vec4(1.0f), glm::vec2(300.0f,300.0f), mesh);

		m_ECS->AddComponent<Relationship>(entity, Relationship());
		m_ECS->AddComponent<RectTransform>(entity, RectTransform(glm::vec3(0.0f), glm::vec2(300.0f)));
		m_ECS->AddComponent<CanvasRenderer>(entity,
			CanvasRenderer(
				m_Specification.Material,
				subTexture,
				glm::vec4(1.0f),
				mesh,
				0,
				true
			));

		setupParent(canvas, entity);
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
		for (size_t i = 0; i < m_CanvasView->Size(); ++i)
		{
			auto& [canvas, renderer, transform] = (*m_CanvasView)[i];
			transform.Size = m_ViewportSize;
			transform.Position = glm::vec3(0.0f);
			transform.Execute<CanvasRendererRebuildEvent>(CanvasRendererRebuildEvent(
				m_CanvasView->GetEntity(i), QuadCanvasRendererRebuild()));
		}
	}
	void GuiContext::SetParent(uint32_t parent, uint32_t child)
	{
		removeParent(child);
		setupParent(parent, child);
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
		for (size_t i = 0; i < m_CheckboxView->Size(); ++i)
		{
			auto& [checkbox,renderer, transform] = (*m_CheckboxView)[i];
			if (checkbox.Checked)
				checkbox.Execute<CheckedEvent>(CheckedEvent{});
		}

		for (size_t i = 0; i < m_LayoutGroup->Size(); ++i)
		{
			auto& [layout, relation, transform] = (*m_LayoutGroup)[i];
			applyLayout(layout, relation, transform);
		}
	}
	void GuiContext::OnRender()
	{
		Renderer::BeginRenderPass(m_RenderPass, false);
		GuiRendererCamera renderCamera;
		renderCamera.Camera = m_Camera;
		renderCamera.ViewMatrix = m_ViewMatrix;

		GuiRenderer::BeginScene(renderCamera);
		for (auto& canvas : m_Canvases)
			submitToRenderer(canvas, glm::vec3(0.0f));


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
		for (int i = 0; i < m_ButtonView->Size(); ++i)
		{
			auto [button, canvasRenderer, rectTransform] = (*m_ButtonView)[i];
			if (Collide(rectTransform.WorldPosition, rectTransform.Size, mousePosition))
			{
				button.Machine.TransitionTo(ButtonState::Clicked);
				SetMeshColor(canvasRenderer.Mesh, canvasRenderer.Color * button.ClickColor);
				if (button.Execute<ClickEvent>(ClickEvent{}))
					return true;
			}
		}
		return false;
	}

	bool GuiContext::buttonOnMouseButtonRelease()
	{
		for (int i = 0; i < m_ButtonView->Size(); ++i)
		{
			auto [button, canvasRenderer, rectTransform] = (*m_ButtonView)[i];
			button.Machine.TransitionTo(ButtonState::Released);
			SetMeshColor(canvasRenderer.Mesh, canvasRenderer.Color);
			if (button.Execute<ReleaseEvent>(ReleaseEvent{}))
				return true;
		}
		return false;
	}

	bool GuiContext::buttonOnMouseMove(const glm::vec2& mousePosition)
	{
		for (int i = 0; i < m_ButtonView->Size(); ++i)
		{
			auto [button,canvasRenderer, rectTransform] = (*m_ButtonView)[i];
			if (Collide(rectTransform.WorldPosition, rectTransform.Size, mousePosition)
				&& button.Machine.TransitionTo(ButtonState::Hoovered))
			{
				SetMeshColor(canvasRenderer.Mesh, canvasRenderer.Color * button.HooverColor);
				button.Execute<HooverEvent>(HooverEvent{});
				return true;
			}
			else if (button.Machine.TransitionTo(ButtonState::UnHoovered))
			{
				SetMeshColor(canvasRenderer.Mesh, canvasRenderer.Color);
				button.Execute<UnHooverEvent>(UnHooverEvent{});
			}
		}
		return false;
	}

	bool GuiContext::checkboxOnMouseButtonPress(const glm::vec2& mousePosition)
	{
		for (int i = 0; i < m_CheckboxView->Size(); ++i)
		{
			auto [checkbox, canvasRenderer, rectTransform] = (*m_CheckboxView)[i];
			if (Collide(rectTransform.WorldPosition, rectTransform.Size, mousePosition))
			{
				if (checkbox.Checked)
				{
					checkbox.Checked = false;
					SetQuadTexCoords(canvasRenderer.Mesh, m_Specification.SubTexture[GuiSpecification::CHECKBOX_UNCHECKED]->GetTexCoords());
					if (checkbox.Execute<UnCheckedEvent>(UnCheckedEvent{}));
						return true;
				}
				else
				{
					checkbox.Checked = true;
					SetQuadTexCoords(canvasRenderer.Mesh, m_Specification.SubTexture[GuiSpecification::CHECKBOX_CHECKED]->GetTexCoords());
					if (checkbox.Execute<CheckedEvent>(CheckedEvent{}));
						return true;
				}
			}
		}
		return false;
	}

	bool GuiContext::checkboxOnMouseMove(const glm::vec2& mousePosition)
	{
		for (int i = 0; i < m_CheckboxView->Size(); ++i)
		{
			auto [checkbox, canvasRenderer, rectTransform] = (*m_CheckboxView)[i];
			if (Collide(rectTransform.WorldPosition, rectTransform.Size, mousePosition))
			{
				SetMeshColor(canvasRenderer.Mesh, canvasRenderer.Color * checkbox.HooverColor);
				checkbox.Execute<HooverEvent>(HooverEvent{});
				return true;
			}
			else
			{
				SetMeshColor(canvasRenderer.Mesh, canvasRenderer.Color);
				checkbox.Execute<UnHooverEvent>(UnHooverEvent{});
			}
		}
		return false;
	}

	void GuiContext::submitToRenderer(uint32_t entity, const glm::vec3& parentPosition)
	{
		auto& rectTransform = m_ECS->GetStorageComponent<RectTransform>(entity);
		rectTransform.WorldPosition = parentPosition + rectTransform.Position;
		auto& canvasRenderer = m_ECS->GetStorageComponent<CanvasRenderer>(entity);
		if (canvasRenderer.IsVisible)
		{
			GuiRenderer::SubmitWidget(&canvasRenderer, &rectTransform);
			auto& currentRel = m_ECS->GetComponent<Relationship>(entity);
			uint32_t currentEntity = currentRel.FirstChild;
			while (currentEntity != NULL_ENTITY)
			{
				submitToRenderer(currentEntity, rectTransform.WorldPosition);
				currentEntity = m_ECS->GetComponent<Relationship>(currentEntity).NextSibling;
			}
		}
	}

	void GuiContext::applyLayout(const LayoutGroup& layout, const Relationship& parentRel, const RectTransform& transform)
	{
		glm::vec2 endOffset = { layout.Padding.Right, layout.Padding.Bottom };
		glm::vec3 position = { layout.Padding.Left - (transform.Size.x / 2.0f), (transform.Size.y / 2.0f) - layout.Padding.Top, 0.0f };


		float maxHeight = 0.0f;
		uint32_t current = parentRel.FirstChild;
		while (current != NULL_ENTITY)
		{
			auto& currentRel = m_ECS->GetComponent<Relationship>(current);
			auto& currentTransform = m_ECS->GetComponent<RectTransform>(current);
			if (currentTransform.Size.y > maxHeight)
				maxHeight = currentTransform.Size.y;


			glm::vec3 sizeOffset = glm::vec3(currentTransform.Size.x / 2.0f, -currentTransform.Size.y / 2.0f, 0.0f);
			if (position.x + sizeOffset.x >= transform.Position.x + transform.Size.x - layout.Padding.Right)
			{
				position.y -= maxHeight + layout.CellSpacing.y;
				position.x = layout.Padding.Left - (transform.Size.x / 2.0f);
				maxHeight = 0.0f;
			}
			currentTransform.Position = position + sizeOffset;
			position.x += currentTransform.Size.x + layout.CellSpacing.x;

			current = currentRel.NextSibling;
		}		
	}
	
	void GuiContext::setupParent(uint32_t parent, uint32_t child)
	{
		auto& parentRel = m_ECS->GetComponent<Relationship>(parent);
		auto& childRel = m_ECS->GetComponent<Relationship>(child);

		childRel.Parent = parent;
		if (parentRel.FirstChild != NULL_ENTITY)
		{
			// Parent has first child
			auto& parentFirstChild = m_ECS->GetComponent<Relationship>(parentRel.FirstChild);	
			// Set new child of parent as previous sibling of first child
			parentFirstChild.PreviousSibling = child;
		
			childRel.NextSibling = parentRel.FirstChild;
			parentRel.FirstChild = child;
		}
		else
		{
			parentRel.FirstChild = child;
		}	
	}
	void GuiContext::removeParent(uint32_t entity)
	{
		auto& childRel = m_ECS->GetComponent<Relationship>(entity);
		auto& parentRel = m_ECS->GetComponent<Relationship>(childRel.Parent);

		// Not siblings ( must be first child of parent )
		if (childRel.NextSibling == NULL_ENTITY && childRel.PreviousSibling == NULL_ENTITY)
		{
			childRel.Parent = NULL_ENTITY;
			parentRel.FirstChild = NULL_ENTITY;
		}

		uint32_t current = parentRel.FirstChild;
		while (current != NULL_ENTITY)
		{
			auto& currentRel = m_ECS->GetComponent<Relationship>(current);			
			if (current == entity)
			{
				if (currentRel.PreviousSibling != NULL_ENTITY)
				{
					auto& prevRel = m_ECS->GetComponent<Relationship>(currentRel.PreviousSibling);
					prevRel.NextSibling = currentRel.NextSibling;
				}
				if (currentRel.NextSibling != NULL_ENTITY)
				{
					auto& nextRel = m_ECS->GetComponent<Relationship>(currentRel.NextSibling);
					nextRel.PreviousSibling = currentRel.PreviousSibling;
				}
				return;
			}
			current = currentRel.NextSibling;
		}
	}

	void TextCanvasRendererRebuild::Rebuild(uint32_t entity, ECS::ECSManager& ecs)
	{	
		auto& transform = ecs.GetComponent<RectTransform>(entity);
		auto& renderer = ecs.GetComponent<CanvasRenderer>(entity);
		auto& text = ecs.GetComponent<Text>(entity);

		size_t oldMeshSize = renderer.Mesh.Vertices.size();
		float height = 0.0f;
		float xCursor = 0.0f;
		float yCursor = 0.0f;
		uint32_t counter = 0;
		for (auto c : text.Source)
		{
			auto& character = text.Font->GetCharacter(c);
			if (xCursor + character.XAdvance >= transform.Size.x)
				break;

			glm::vec2 charSize = {
				character.X1Coord - character.X0Coord,
				character.Y1Coord - character.Y0Coord
			};
			if (height < charSize.y) height = charSize.y;

			glm::vec2 charOffset = { character.XOffset, charSize.y - character.YOffset };
			glm::vec2 charPosition = { xCursor + charOffset.x, yCursor - charOffset.y };
			glm::vec4 charTexCoord = {
				(float)character.X0Coord / (float)text.Font->GetWidth(), (float)character.Y0Coord / (float)text.Font->GetHeight(),
				(float)character.X1Coord / (float)text.Font->GetWidth(), (float)character.Y1Coord / (float)text.Font->GetHeight()
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
				renderer.Mesh.Vertices.push_back(Vertex{ text.Color, quads[i], texCoords[i] });

			xCursor += character.XAdvance;
			counter++;
		}
		if (text.Alignment == TextAlignment::Center)
		{
			for (size_t i = oldMeshSize; i < renderer.Mesh.Vertices.size(); ++i)
			{
				renderer.Mesh.Vertices[i].Position.x -= xCursor / 2.0f;
				renderer.Mesh.Vertices[i].Position.y -= height / 2.0f;
			}
		}
	}

	void QuadCanvasRendererRebuild::Rebuild(uint32_t entity, ECS::ECSManager& ecs)
	{
		constexpr size_t quadVertexCount = 4;
		auto& transform = ecs.GetComponent<RectTransform>(entity);
		auto& renderer = ecs.GetComponent<CanvasRenderer>(entity);
		auto& texCoord = renderer.SubTexture->GetTexCoords();

		glm::vec2 texCoords[quadVertexCount] = {
			{texCoord.x,texCoord.y},
			{texCoord.z,texCoord.y},
			{texCoord.z,texCoord.w},
			{texCoord.x,texCoord.w}
		};
		glm::vec3 vertices[quadVertexCount] = {
			{  -transform.Size.x / 2.0f,  -transform.Size.y / 2.0f, 0.0f},
			{   transform.Size.x / 2.0f,  -transform.Size.y / 2.0f, 0.0f},
			{   transform.Size.x / 2.0f,   transform.Size.y / 2.0f, 0.0f},
			{  -transform.Size.x / 2.0f,   transform.Size.y / 2.0f, 0.0f}
		};
		for (size_t i = 0; i < quadVertexCount; ++i)
			renderer.Mesh.Vertices.push_back(Vertex{ renderer.Color, vertices[i], texCoords[i] });
	}
}