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
		if (!source)
			return;

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

		FrameBufferSpecs fboSpecs;
		fboSpecs.ClearColor = { 0.1f,0.1f,0.1f,1.0f };
		fboSpecs.Attachments = {
			FrameBufferTextureSpecs(FrameBufferTextureFormat::RGBA16F),
			FrameBufferTextureSpecs(FrameBufferTextureFormat::DEPTH24STENCIL8)
		};
		fboSpecs.SwapChainTarget = true;

		Ref<FrameBuffer> fbo = FrameBuffer::Create(fboSpecs);
		m_RenderPass = RenderPass::Create({ fbo });

		m_LineRenderView = &m_ECS->CreateView<LineRenderer>();
		m_RenderView = &m_ECS->CreateView<CanvasRenderer, RectTransform>();
		m_CanvasView = &m_ECS->CreateView<Canvas, CanvasRenderer, RectTransform>();
		m_ButtonView = &m_ECS->CreateView<Button, CanvasRenderer, RectTransform>();
		m_CheckboxView = &m_ECS->CreateView<Checkbox, CanvasRenderer, RectTransform>();
		m_SliderView = &m_ECS->CreateView<Slider, CanvasRenderer, RectTransform>();
		m_LayoutGroupView = &m_ECS->CreateView<LayoutGroup, Relationship, RectTransform>();
		m_LayoutView = &m_ECS->CreateView<Layout, Relationship, CanvasRenderer, RectTransform>();
		m_InputFieldView = &m_ECS->CreateView<InputField, CanvasRenderer, RectTransform>();

		for (uint32_t i = 0; i < ecs->GetNumberOfEntities(); ++i)
		{
			if (ecs->Contains<Canvas>(i))
			{
				m_Canvases.push_back(i);
			}
		}
	}

	bool GuiContext::onQuadRectTransformResized(ComponentResizedEvent& event)
	{
		auto entity = event.GetEntity();
		constexpr size_t quadVertexCount = 4;
		auto& transform = entity.GetComponent<RectTransform>();
		auto& renderer = entity.GetComponent<CanvasRenderer>();
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
		return false;
	}

	bool GuiContext::onTextRectTransformResized(ComponentResizedEvent& event)
	{
		auto& entity = event.GetEntity();
		auto& transform = entity.GetComponent<RectTransform>();
		auto& renderer = entity.GetComponent<CanvasRenderer>();
		auto& text = entity.GetComponent<Text>();

		size_t oldMeshSize = renderer.Mesh.Vertices.size();
		int32_t height = 0.0f;
		int32_t xCursor = 0.0f;
		int32_t yCursor = 0.0f;
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
			if (height < character.Y1Coord - character.Y0Coord)
				height = character.Y1Coord - character.Y0Coord;

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
			int32_t xOffset = xCursor / 2;
			int32_t yOffset = height / 2;
			for (size_t i = oldMeshSize; i < renderer.Mesh.Vertices.size(); ++i)
			{
				renderer.Mesh.Vertices[i].Position.x -= xOffset;
				renderer.Mesh.Vertices[i].Position.y -= yOffset;
			}
		}
		return false;
	}

	Entity GuiContext::CreateCanvas(const CanvasSpecification& specs)
	{
		auto& texCoords = m_Specification.SubTexture[GuiSpecification::BUTTON]->GetTexCoords();
		uint32_t entity = m_ECS->CreateEntity();
		m_ECS->AddComponent<IDComponent>(entity, IDComponent());

		Mesh mesh;
		GenerateQuadMesh(texCoords, specs.Color, specs.Size, mesh);
	
		m_ECS->AddComponent<Relationship>(entity, Relationship());
		m_ECS->AddComponent<Canvas>(entity, Canvas(specs.RenderMode, specs.Color));
		auto& transform = m_ECS->AddComponent<RectTransform>(entity, RectTransform(specs.Position, specs.Size));
		transform.RegisterCallback<ComponentResizedEvent>(Hook(&GuiContext::onQuadRectTransformResized, this));
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
		return { entity, m_ECS };
	}
	Entity GuiContext::CreatePanel(uint32_t parent, const PanelSpecification& specs)
	{
		auto& texCoords = m_Specification.SubTexture[GuiSpecification::BUTTON]->GetTexCoords();
		uint32_t entity = m_ECS->CreateEntity();
		m_ECS->AddComponent<IDComponent>(entity, IDComponent());

		Mesh mesh;
		GenerateQuadMesh(texCoords, specs.Color, specs.Size, mesh);

		m_ECS->AddComponent<Relationship>(entity, Relationship());
		auto& transform = m_ECS->AddComponent<RectTransform>(entity, RectTransform(specs.Position, specs.Size));
		m_ECS->AddComponent<CanvasRenderer>(entity,
			CanvasRenderer(
				m_Specification.Material,
				m_Specification.SubTexture[GuiSpecification::BUTTON],
				specs.Color,
				mesh,
				0,
				true
			));
		
		transform.RegisterCallback<ComponentResizedEvent>(Hook(&GuiContext::onQuadRectTransformResized, this));

		Relationship::SetupRelation(parent, entity, *m_ECS);
		return { entity, m_ECS };
	}
	Entity GuiContext::CreateButton(uint32_t parent, const ButtonSpecification& specs)
	{
		auto& texCoords = m_Specification.SubTexture[GuiSpecification::BUTTON]->GetTexCoords();
		uint32_t entity = m_ECS->CreateEntity();
		m_ECS->AddComponent<IDComponent>(entity, IDComponent());

		Mesh mesh;
		GenerateQuadMesh(texCoords, specs.DefaultColor, specs.Size, mesh);

		m_ECS->AddComponent<Relationship>(entity, Relationship());
		auto& transform = m_ECS->AddComponent<RectTransform>(entity, RectTransform( specs.Position, specs.Size));
		transform.RegisterCallback<ComponentResizedEvent>(Hook(&GuiContext::onQuadRectTransformResized, this));
		m_ECS->AddComponent<CanvasRenderer>( entity,
			CanvasRenderer(
				m_Specification.Material,
				m_Specification.SubTexture[GuiSpecification::BUTTON],
				specs.DefaultColor,
				mesh,
				specs.SortLayer,
				true
		));

		m_ECS->AddComponent<Button>(entity, Button(specs.ClickColor, specs.HooverColor));
			
		Relationship::SetupRelation(parent, entity, *m_ECS);
		
		uint32_t textEntity = m_ECS->CreateEntity();
		m_ECS->AddComponent<IDComponent>(textEntity, IDComponent());

		Mesh textMesh;
		GenerateTextMesh(specs.Name.c_str(), m_Specification.Font, specs.DefaultColor, specs.Size, textMesh, TextAlignment::Center);
		auto& textRectTransform = m_ECS->AddComponent<RectTransform>(textEntity, RectTransform(glm::vec3(0.0f), specs.Size));
		
		textRectTransform.RegisterCallback<ComponentResizedEvent>(Hook(&GuiContext::onTextRectTransformResized, this));
		m_ECS->AddComponent<Relationship>(textEntity, Relationship());

		m_ECS->AddComponent<CanvasRenderer>(textEntity,
			CanvasRenderer(
				m_Specification.Material,
				m_Specification.SubTexture[GuiSpecification::FONT],
				specs.DefaultColor,
				textMesh,
				specs.SortLayer + 1,
				true
		));
		
		m_ECS->AddComponent<Text>(textEntity,
			Text(
			specs.Name.c_str(),
			m_Specification.Font,
			specs.DefaultColor,
			TextAlignment::Center
		));
		
		Relationship::SetupRelation(entity, textEntity, *m_ECS);
		return { entity, m_ECS };
	}
	Entity GuiContext::CreateCheckbox(uint32_t parent, const CheckboxSpecification& specs)
	{
		uint32_t entity = m_ECS->CreateEntity();
		m_ECS->AddComponent<IDComponent>(entity, IDComponent());
		auto& checkbox = m_ECS->AddComponent<Checkbox>(entity, Checkbox(specs.HooverColor));


		glm::vec4 texCoords = m_Specification.SubTexture[GuiSpecification::CHECKBOX_UNCHECKED]->GetTexCoords();
		if (checkbox.Checked)
			texCoords = m_Specification.SubTexture[GuiSpecification::CHECKBOX_CHECKED]->GetTexCoords();

		Mesh mesh;
		GenerateQuadMesh(texCoords, specs.DefaultColor, specs.Size, mesh);
		
		m_ECS->AddComponent<Relationship>(entity, Relationship());
		auto& transform = m_ECS->AddComponent<RectTransform>(entity, RectTransform(specs.Position, specs.Size));
		transform.RegisterCallback<ComponentResizedEvent>(Hook(&GuiContext::onQuadRectTransformResized, this));
		m_ECS->AddComponent<CanvasRenderer>(entity,
			CanvasRenderer(
				m_Specification.Material,
				m_Specification.SubTexture[GuiSpecification::CHECKBOX_UNCHECKED],
				specs.DefaultColor,
				mesh,
				0,
				true
			));


		Relationship::SetupRelation(parent, entity, *m_ECS);


		uint32_t textEntity = m_ECS->CreateEntity();
		m_ECS->AddComponent<IDComponent>(textEntity, IDComponent());
		Mesh textMesh;
		GenerateTextMesh(specs.Name.c_str(), m_Specification.Font, specs.DefaultColor, specs.Size, textMesh, TextAlignment::Center);
		auto& textRectTransform = m_ECS->AddComponent<RectTransform>(textEntity, RectTransform(glm::vec3(0.0f), glm::vec2(1.0f)));
		textRectTransform.RegisterCallback<ComponentResizedEvent>(Hook(&GuiContext::onTextRectTransformResized, this));
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
				specs.Name.c_str(),
				m_Specification.Font,
				specs.DefaultColor,
				TextAlignment::Center
			));
		
		Relationship::SetupRelation(entity,textEntity, *m_ECS);

		return { entity, m_ECS };
	}
	Entity GuiContext::CreateSlider(uint32_t parent, const SliderSpecification& specs)
	{
		uint32_t entity = m_ECS->CreateEntity();
		m_ECS->AddComponent<IDComponent>(entity, IDComponent());
		auto& slider = m_ECS->AddComponent<Slider>(entity, Slider(specs.HooverColor));

		glm::vec4 texCoords = m_Specification.SubTexture[GuiSpecification::BUTTON]->GetTexCoords();
	
		Mesh mesh;
		GenerateQuadMesh(texCoords, specs.DefaultColor, specs.Size, mesh);

		m_ECS->AddComponent<Relationship>(entity, Relationship());
		auto & transform = m_ECS->AddComponent<RectTransform>(entity, RectTransform(specs.Position, specs.Size));
		transform.RegisterCallback<ComponentResizedEvent>(Hook(&GuiContext::onQuadRectTransformResized, this));
		m_ECS->AddComponent<CanvasRenderer>(entity,
			CanvasRenderer(
				m_Specification.Material,
				m_Specification.SubTexture[GuiSpecification::BUTTON],
				specs.DefaultColor,
				mesh,
				specs.SortLayer,
				true
			));


		Relationship::SetupRelation(parent, entity, *m_ECS);


		uint32_t handle = m_ECS->CreateEntity();
		m_ECS->AddComponent<IDComponent>(handle, IDComponent());
		Mesh handleMesh;
		GenerateQuadMesh(texCoords, specs.HandleColor, specs.HandleSize, handleMesh);
		m_ECS->AddComponent<Relationship>(handle, Relationship());
		auto& handleRectTransform = m_ECS->AddComponent<RectTransform>(handle, RectTransform(glm::vec3(0.0f), specs.HandleSize));
		handleRectTransform.RegisterCallback<ComponentResizedEvent>(Hook(&GuiContext::onQuadRectTransformResized, this));
		m_ECS->AddComponent<CanvasRenderer>(handle,
			CanvasRenderer(
				m_Specification.Material,
				m_Specification.SubTexture[GuiSpecification::BUTTON],
				specs.HandleColor,
				handleMesh,
				specs.SortLayer+1,
				true
			));
		

		

		uint32_t textEntity = m_ECS->CreateEntity();
		m_ECS->AddComponent<IDComponent>(textEntity, IDComponent());
		Mesh textMesh;
		GenerateTextMesh(specs.Name.c_str(), m_Specification.Font, specs.DefaultColor, specs.Size, textMesh, TextAlignment::Center);
		auto& textRectTransform = m_ECS->AddComponent<RectTransform>(textEntity, RectTransform(glm::vec3(0.0f), glm::vec2(1.0f)));
		textRectTransform.RegisterCallback<ComponentResizedEvent>(Hook(&GuiContext::onTextRectTransformResized, this));
		m_ECS->AddComponent<Relationship>(textEntity, Relationship());

		m_ECS->AddComponent<CanvasRenderer>(textEntity,
			CanvasRenderer(
				m_Specification.Material,
				m_Specification.SubTexture[GuiSpecification::FONT],
				specs.DefaultColor,
				textMesh,
				specs.SortLayer+1,
				true
			));

		m_ECS->AddComponent<Text>(textEntity,
			Text(
				specs.Name.c_str(),
				m_Specification.Font,
				specs.DefaultColor,
				TextAlignment::Center
			));

		Relationship::SetupRelation(entity, textEntity, *m_ECS);
		Relationship::SetupRelation(entity, handle, *m_ECS);

		return { entity, m_ECS };
	}
	Entity GuiContext::CreateText(uint32_t parent, const TextSpecification& specs)
	{
		uint32_t entity = m_ECS->CreateEntity();
		m_ECS->AddComponent<IDComponent>(entity, IDComponent());

		Mesh mesh;
		GenerateTextMesh(specs.Source.c_str(), m_Specification.Font, specs.Color, specs.Size, mesh, specs.Alignment);

		auto& textRectTransform = m_ECS->AddComponent<RectTransform>(entity, RectTransform(specs.Position, specs.Size));
		textRectTransform.RegisterCallback<ComponentResizedEvent>(Hook(&GuiContext::onTextRectTransformResized, this));
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
				specs.Source.c_str(),
				m_Specification.Font,
				specs.Color,
				specs.Alignment
			));

		
		Relationship::SetupRelation(parent, entity, *m_ECS);

		return { entity, m_ECS };
	}
	Entity GuiContext::CreateImage(uint32_t canvas, const ImageSpecification& specs)
	{
		auto& texCoords = specs.SubTexture->GetTexCoords();
		uint32_t entity = m_ECS->CreateEntity();
		m_ECS->AddComponent<IDComponent>(entity, IDComponent());

		Mesh mesh;
		GenerateQuadMesh(texCoords, specs.Color, specs.Size, mesh);

		m_ECS->AddComponent<Relationship>(entity, Relationship());
		auto & transform = m_ECS->AddComponent<RectTransform>(entity, RectTransform(specs.Position, specs.Size));
		
		m_ECS->AddComponent<CanvasRenderer>(entity,
			CanvasRenderer(
				m_Specification.Material,
				specs.SubTexture,
				specs.Color,
				mesh,
				0,
				true
			));

		transform.RegisterCallback<ComponentResizedEvent>(Hook(&GuiContext::onQuadRectTransformResized, this));
		Relationship::SetupRelation(canvas, entity, *m_ECS);

		return { entity, m_ECS };
	}
	Entity GuiContext::CreateInputField(uint32_t parent, const InputFieldSpecification& specs)
	{
		uint32_t textEntity = m_ECS->CreateEntity();
		m_ECS->AddComponent<IDComponent>(textEntity, IDComponent());
		Mesh textMesh;
		GenerateTextMesh(nullptr, m_Specification.Font, specs.DefaultColor, specs.Size, textMesh, TextAlignment::Center);
		auto& textRectTransform = m_ECS->AddComponent<RectTransform>(textEntity, RectTransform(glm::vec3(0.0f), specs.Size));
		textRectTransform.RegisterCallback<ComponentResizedEvent>(Hook(&GuiContext::onTextRectTransformResized, this));
		m_ECS->AddComponent<Relationship>(textEntity, Relationship());

		m_ECS->AddComponent<CanvasRenderer>(textEntity,
			CanvasRenderer(
				m_Specification.Material,
				m_Specification.SubTexture[GuiSpecification::FONT],
				specs.DefaultColor,
				textMesh,
				specs.SortLayer + 1,
				true
			));

		m_ECS->AddComponent<Text>(textEntity,
			Text(
				"",
				m_Specification.Font,
				specs.DefaultColor,
				TextAlignment::Center
			));

		
		uint32_t entity = m_ECS->CreateEntity();
		m_ECS->AddComponent<IDComponent>(entity, IDComponent());

		auto& field = m_ECS->AddComponent<InputField>(entity, InputField(specs.SelectColor, specs.HooverColor, textEntity, m_ECS));

		glm::vec4 texCoords = m_Specification.SubTexture[GuiSpecification::BUTTON]->GetTexCoords();

		Mesh mesh;
		GenerateQuadMesh(texCoords, specs.DefaultColor, specs.Size, mesh);

		m_ECS->AddComponent<Relationship>(entity, Relationship());
		auto & transform = m_ECS->AddComponent<RectTransform>(entity, RectTransform(specs.Position, specs.Size));
		transform.RegisterCallback<ComponentResizedEvent>(Hook(&GuiContext::onQuadRectTransformResized, this));
		m_ECS->AddComponent<CanvasRenderer>(entity,
			CanvasRenderer(
				m_Specification.Material,
				m_Specification.SubTexture[GuiSpecification::BUTTON],
				specs.DefaultColor,
				mesh,
				specs.SortLayer,
				true
			));


		Relationship::SetupRelation(parent, entity, *m_ECS);
		Relationship::SetupRelation(entity, textEntity, *m_ECS);

		return { entity, m_ECS };
	}
	void GuiContext::SetViewportSize(uint32_t width, uint32_t height)
	{
		m_RenderPass->GetSpecification().TargetFramebuffer->Resize(width, height);

		float w = (float)width;
		float h = (float)height;

		m_Camera.SetProjectionMatrix(glm::ortho(-w * 0.5f, w * 0.5f, -h * 0.5f, h * 0.5f));
		m_ViewportSize = { w, h };

		for (size_t i = 0; i < m_CanvasView->Size(); ++i)
		{
			auto& [canvas, renderer, transform] = (*m_CanvasView)[i];
			transform.Size = m_ViewportSize;
			transform.Position = glm::vec3(0.0f);
			transform.Execute<ComponentResizedEvent>(ComponentResizedEvent({ m_CanvasView->GetEntity(i), m_ECS }));
		}
		
	}
	void GuiContext::SetParent(uint32_t parent, uint32_t child)
	{
		Relationship::RemoveRelation(child, *m_ECS);
		Relationship::SetupRelation(parent, child, *m_ECS);
	}
	void GuiContext::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);

		if (dispatcher.Dispatch<WindowResizeEvent>(Hook(&GuiContext::onWindowResize, this)))
		{
		}
		if (dispatcher.Dispatch<MouseButtonPressEvent>(Hook(&GuiContext::onMouseButtonPress, this)))
		{
		}
		else if (dispatcher.Dispatch<MouseButtonReleaseEvent>(Hook(&GuiContext::onMouseButtonRelease, this)))
		{
		}
		else if (dispatcher.Dispatch<MouseMovedEvent>(Hook(&GuiContext::onMouseMove, this)))
		{
		}
		else if (dispatcher.Dispatch<KeyTypedEvent>(Hook(&GuiContext::onKeyTyped, this)))
		{
		}
		else if (dispatcher.Dispatch<KeyPressedEvent>(Hook(&GuiContext::onKeyPressed, this)))
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

		for (auto& canvas : m_Canvases)
			updateTransform(canvas, glm::vec3(0.0f));

		for (size_t i = 0; i < m_LayoutGroupView->Size(); ++i)
		{
			auto& [layout, relation, transform] = (*m_LayoutGroupView)[i];
			applyLayoutGroup(layout, relation, transform);
		}
	}
	void GuiContext::OnRender()
	{
		Renderer::BeginRenderPass(m_RenderPass, false);
		GuiRendererCamera renderCamera;
		renderCamera.Camera = m_Camera;
		renderCamera.ViewMatrix = m_ViewMatrix;

		GuiRenderer::BeginScene(renderCamera, m_ViewportSize);
		
		for (size_t i = 0; i < m_RenderView->Size(); ++i)
		{
			auto &[canvasRenderer, rectTransform] = (*m_RenderView)[i];
			GuiRenderer::SubmitWidget(&canvasRenderer, &rectTransform);
		}
		for (size_t i = 0; i < m_LineRenderView->Size(); ++i)
		{
			auto& [lineRenderer] = (*m_LineRenderView)[i];
			GuiRenderer::SubmitWidget(&lineRenderer);
		}
		GuiRenderer::EndScene();
		Renderer::EndRenderPass();
		Renderer::WaitAndRender();
	}

	bool GuiContext::onMouseButtonPress(MouseButtonPressEvent& event)
	{
		auto [mx, my] = Input::GetMousePosition();
		glm::vec2 mousePosition = MouseToWorld({ mx,my }, m_ViewportSize);
		if (event.IsButtonPressed(MouseCode::XYZ_MOUSE_BUTTON_LEFT))
		{	
			if (inputFieldOnMouseButtonPress(mousePosition))
				return true;
			else if (buttonOnMouseButtonPress(mousePosition))
				return true;
			else if (checkboxOnMouseButtonPress(mousePosition))
				return true;
			else if (sliderOnMouseButtonPress(mousePosition))
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
			else if (sliderOnMouseButtonRelease())
				return true;
		}
		return false;
	}
	bool GuiContext::onWindowResize(WindowResizeEvent& event)
	{
		SetViewportSize((uint32_t)event.GetWidth(), (uint32_t)event.GetHeight());
		return false;
	}
	bool GuiContext::onMouseMove(MouseMovedEvent& event)
	{
		auto [mx, my] = Input::GetMousePosition();
		glm::vec2 mousePosition = MouseToWorld({ mx,my }, m_ViewportSize);
		
		if (inputFieldOnMouseMove(mousePosition))
			return true;
		else if (buttonOnMouseMove(mousePosition))
			return true;
		else if (checkboxOnMouseMove(mousePosition))
			return true;
		else if (sliderOnMouseMove(mousePosition))
			return true;

		return false;
	}

	bool GuiContext::onKeyPressed(KeyPressedEvent& event)
	{
		if (event.IsKeyPressed(KeyCode::XYZ_KEY_BACKSPACE))
		{
			for (size_t i = 0; i < m_InputFieldView->Size(); ++i)
			{
				auto& [inputField, canvasRenderer, rectTransform] = (*m_InputFieldView)[i];
				if (inputField.Machine.GetCurrentState().GetID() == InputFieldState::Selected)
				{
					if (inputField.ECS && inputField.ECS->Contains<Text>(inputField.TextEntity))
					{
						auto& text = inputField.ECS->GetStorageComponent<Text>(inputField.TextEntity);
						auto& textRectTransform = inputField.ECS->GetStorageComponent<RectTransform>(inputField.TextEntity);

						if (!text.Source.empty())
						{
							text.Source.pop_back();
							textRectTransform.Execute<ComponentResizedEvent>(ComponentResizedEvent({ m_CanvasView->GetEntity(i), m_ECS }));
						}
						return true;
					}
				}
			}
		}
		return false;
	}

	bool GuiContext::onKeyTyped(KeyTypedEvent& event)
	{
		for (size_t i = 0; i < m_InputFieldView->Size(); ++i)
		{
			auto& [inputField, canvasRenderer, rectTransform] = (*m_InputFieldView)[i];
			if (inputField.Machine.GetCurrentState().GetID() == InputFieldState::Selected)
			{
				if (inputField.ECS && inputField.ECS->Contains<Text>(inputField.TextEntity))
				{
					auto& text = inputField.ECS->GetStorageComponent<Text>(inputField.TextEntity);
					auto& textRectTransform = inputField.ECS->GetStorageComponent<RectTransform>(inputField.TextEntity);


					text.Source += event.GetKey();
					textRectTransform.Execute<ComponentResizedEvent>(ComponentResizedEvent({ m_CanvasView->GetEntity(i), m_ECS }));
					return true;
				}
			}
		}
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
				if (button.Execute<ClickEvent>(ClickEvent{{m_ButtonView->GetEntity(i), m_ECS} }))
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
			SetMeshColor(canvasRenderer.Mesh, canvasRenderer.Color);
			if (button.Machine.TransitionTo(ButtonState::Released))
			{
				if (button.Execute<ReleaseEvent>(ReleaseEvent{ {m_ButtonView->GetEntity(i), m_ECS} }))
					return true;
			}
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

	bool GuiContext::sliderOnMouseButtonPress(const glm::vec2& mousePosition)
	{
		for (int i = 0; i < m_SliderView->Size(); ++i)
		{
			auto [slider, canvasRenderer, rectTransform] = (*m_SliderView)[i];
			if (Collide(rectTransform.WorldPosition, rectTransform.Size, mousePosition))
			{
				slider.Machine.TransitionTo(SliderState::Dragged);
				SetMeshColor(canvasRenderer.Mesh, canvasRenderer.Color * slider.HooverColor);
				if (slider.Execute<ClickEvent>(ClickEvent{ {m_SliderView->GetEntity(i), m_ECS} }))
					return true;
			}
		}
		return false;
	}

	bool GuiContext::sliderOnMouseButtonRelease()
	{
		for (int i = 0; i < m_SliderView->Size(); ++i)
		{
			auto [slider, canvasRenderer, rectTransform] = (*m_SliderView)[i];
			SetMeshColor(canvasRenderer.Mesh, canvasRenderer.Color);
			if (slider.Machine.TransitionTo(SliderState::Released))
			{
				if (slider.Execute<ReleaseEvent>(ReleaseEvent{ {m_SliderView->GetEntity(i), m_ECS} }))
					return true;
			}
		}
		return false;
	}

	bool GuiContext::sliderOnMouseMove(const glm::vec2& mousePosition)
	{
		for (int i = 0; i < m_SliderView->Size(); ++i)
		{
			auto [slider, canvasRenderer, rectTransform] = (*m_SliderView)[i];
			if (slider.Machine.GetCurrentState().GetID() == SliderState::Dragged)
			{
				auto& rel = m_ECS->GetStorageComponent<Relationship>(m_SliderView->GetEntity(i));
				uint32_t handle = rel.FirstChild;
				auto& handleTransform = m_ECS->GetStorageComponent<RectTransform>(handle);
				float diff = mousePosition.x - handleTransform.WorldPosition.x;
				
				if (handleTransform.Position.x + (handleTransform.Size.x / 2.0f) + diff < (rectTransform.Size.x / 2.0f)
				&& (handleTransform.Position.x - (handleTransform.Size.x / 2.0f) + diff > (-rectTransform.Size.x / 2.0f)))
				{
					handleTransform.Position.x += diff;
				}
				else if (handleTransform.Position.x < 0.0f)
				{
					handleTransform.Position.x = (-rectTransform.Size.x / 2.0f) + (handleTransform.Size.x / 2.0f);
				}
				else
				{
					handleTransform.Position.x = (rectTransform.Size.x / 2.0f) - (handleTransform.Size.x / 2.0f);
				}
				
				slider.Value = handleTransform.Position.x  / (rectTransform.Size.x - handleTransform.Size.x) + 0.5f;
				slider.Execute<DraggedEvent>(DraggedEvent({ m_SliderView->GetEntity(i), m_ECS }, slider.Value));
				handleTransform.Execute<ComponentResizedEvent>(ComponentResizedEvent({ m_CanvasView->GetEntity(i), m_ECS }));
				return true;
			}
		}
		return false;
	}

	bool GuiContext::inputFieldOnMouseButtonPress(const glm::vec2& mousePosition)
	{
		for (int i = 0; i < m_InputFieldView->Size(); ++i)
		{
			auto [inputField, canvasRenderer, rectTransform] = (*m_InputFieldView)[i];
			if (inputField.Machine.GetCurrentState().GetID() == InputFieldState::Selected)
			{
				inputField.Machine.TransitionTo(InputFieldState::Released);
				SetMeshColor(canvasRenderer.Mesh, canvasRenderer.Color);
				inputField.Execute<ReleaseEvent>(ReleaseEvent({ m_InputFieldView->GetEntity(i), m_ECS }));
			}
			if (Collide(rectTransform.WorldPosition, rectTransform.Size, mousePosition))
			{
				inputField.Machine.TransitionTo(InputFieldState::Hoovered);
				if (inputField.Machine.TransitionTo(InputFieldState::Selected))
				{
					SetMeshColor(canvasRenderer.Mesh, canvasRenderer.Color * inputField.SelectColor);
					inputField.Execute<ClickEvent>(ClickEvent({ m_InputFieldView->GetEntity(i) , m_ECS }));
					return true;
				}
			}
		}
		return false;
	}

	bool GuiContext::inputFieldOnMouseMove(const glm::vec2& mousePosition)
	{
		for (int i = 0; i < m_InputFieldView->Size(); ++i)
		{
			auto [inputField, canvasRenderer, rectTransform] = (*m_InputFieldView)[i];
			if (Collide(rectTransform.WorldPosition, rectTransform.Size, mousePosition))
			{
				if (inputField.Machine.TransitionTo(InputFieldState::Hoovered))
				{
					SetMeshColor(canvasRenderer.Mesh, canvasRenderer.Color * inputField.HooverColor);
					inputField.Execute<HooverEvent>(HooverEvent{});
				}
				return true;
			}
			else if (inputField.Machine.TransitionTo(InputFieldState::UnHoovered))
			{
				SetMeshColor(canvasRenderer.Mesh, canvasRenderer.Color);
				inputField.Execute<UnHooverEvent>(UnHooverEvent{});
			}
		}
		return false;
	}

	void GuiContext::updateTransform(uint32_t entity, const glm::vec3& parentPosition)
	{
		auto& rectTransform = m_ECS->GetStorageComponent<RectTransform>(entity);
		rectTransform.WorldPosition = parentPosition + rectTransform.Position;
		auto& canvasRenderer = m_ECS->GetStorageComponent<CanvasRenderer>(entity);
		if (canvasRenderer.IsVisible)
		{		
			auto& currentRel = m_ECS->GetComponent<Relationship>(entity);
			uint32_t currentEntity = currentRel.FirstChild;
			while (currentEntity != NULL_ENTITY)
			{
				updateTransform(currentEntity, rectTransform.WorldPosition);
				currentEntity = m_ECS->GetComponent<Relationship>(currentEntity).NextSibling;
			}
		}
	}

	
	void GuiContext::applyLayoutGroup(const LayoutGroup& layout, const Relationship& parentRel, const RectTransform& transform)
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
			if (position.x + (sizeOffset.x * 2.0f) >= (transform.Size.x / 2.0f) - layout.Padding.Right)
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
}