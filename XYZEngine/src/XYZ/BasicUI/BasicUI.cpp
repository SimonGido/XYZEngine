#include "stdafx.h"
#include "BasicUI.h"
#include "BasicUIInput.h"
#include "BasicUIHelper.h"

#include "XYZ/Renderer/Renderer2D.h"
#include "XYZ/Renderer/Renderer.h"
#include "XYZ/Core/Application.h"
#include "XYZ/Core/Input.h"

#include <glm/gtx/transform.hpp>

namespace XYZ {

	static bUIContext* s_Context = nullptr;

	void bUI::Init()
	{
		s_Context = new bUIContext();
		auto& app = Application::Get();
		s_Context->ViewportSize = glm::vec2(app.GetWindow().GetWidth(), 
											app.GetWindow().GetHeight());
	}
	void bUI::Shutdown()
	{
		delete s_Context;
	}
	void bUI::Update()
	{
		s_Context->Renderer.Begin();
		s_Context->Data.Update();
		s_Context->Data.BuildMesh(s_Context->Renderer);

		glm::mat4 viewMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f));
		viewMatrix = glm::inverse(viewMatrix);
		Renderer2D::BeginScene(glm::ortho(0.0f, s_Context->ViewportSize.x, s_Context->ViewportSize.y, 0.0f) * viewMatrix);
		if (s_Context->Renderer.GetMesh().Scissors.size())
		{
			s_Context->Renderer.UpdateScissorBuffer(s_Context->Config.m_ScissorBuffer);
			s_Context->Config.m_ScissorBuffer->BindRange(0, s_Context->Renderer.GetMesh().Scissors.size() * sizeof(bUIScissor), 0);
		}
		Renderer2D::SetMaterial(s_Context->Config.m_Material);
		s_Context->Renderer.BindCustomTextures();

		for (const bUIQuad& quad : s_Context->Renderer.GetMesh().Quads)
			Renderer2D::SubmitQuadNotCentered(quad.Position, quad.Size, quad.TexCoord, quad.TextureID, quad.Color, (float)quad.ScissorID);
		for (const bUILine& line : s_Context->Renderer.GetMesh().Lines)
			Renderer2D::SubmitLine(line.P0, line.P1, line.Color);
		
		Renderer2D::Flush();
		Renderer2D::FlushLines();
		Renderer2D::EndScene();
		Renderer::WaitAndRender();
	}
	void bUI::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<WindowResizeEvent>(&onWindowResize);
		dispatcher.Dispatch<MouseButtonPressEvent>(&onMouseButtonPress);
		dispatcher.Dispatch<MouseButtonReleaseEvent>(&onMouseButtonRelease);
		dispatcher.Dispatch<MouseMovedEvent>(&onMouseMove);
		dispatcher.Dispatch<MouseScrollEvent>(&onMouseScroll);
		dispatcher.Dispatch<KeyTypedEvent>(&onKeyType);
		dispatcher.Dispatch<KeyPressedEvent>(&onKeyPress);
	}
	void bUI::SetupLayout(const std::string& uiName, const std::string& name, const bUILayout& layout)
	{
		bUIAllocator& allocator = s_Context->Data.GetAllocator(uiName);
		bUIElement* element = allocator.GetElement<bUIElement>(name);
		bUIHelper::ResolvePosition(element->ID, allocator.m_Tree, layout);
	}

	bUIConfig& bUI::GetConfig()
	{
		return s_Context->Config;
	}
	const bUIContext& bUI::GetContext()
	{
		return *s_Context;
	}
	bool bUI::onWindowResize(WindowResizeEvent& event)
	{
		glm::vec2 newViewportSize((float)event.GetWidth(), (float)event.GetHeight());
		s_Context->ViewportSize = newViewportSize;
		return false;
	}
	bool bUI::onMouseButtonPress(MouseButtonPressEvent& event)
	{
		return bUIInput::OnMouseButtonPress(event, s_Context->EditData, s_Context->Data);
	}
	bool bUI::onMouseButtonRelease(MouseButtonReleaseEvent& event)
	{
		return bUIInput::OnMouseButtonRelease(event, s_Context->EditData, s_Context->Data);
	}
	bool bUI::onMouseMove(MouseMovedEvent& event)
	{
		return bUIInput::OnMouseMove(event, s_Context->EditData, s_Context->Data);
	}
	bool bUI::onMouseScroll(MouseScrollEvent& event)
	{
		return bUIInput::OnMouseScroll(event, s_Context->EditData, s_Context->Data);
	}
	bool bUI::onKeyType(KeyTypedEvent& event)
	{
		return bUIInput::OnKeyType(event);
	}
	bool bUI::onKeyPress(KeyPressedEvent& event)
	{
		return bUIInput::OnKeyPress(event);
	}
	bUIContext& bUI::getContext()
	{
		return *s_Context;
	}
}