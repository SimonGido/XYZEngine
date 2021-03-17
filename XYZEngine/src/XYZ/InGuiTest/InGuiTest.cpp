#include "stdafx.h"
#include "InGuiTest.h"


#include "XYZ/Core/Input.h"
#include "XYZ/Renderer/Renderer2D.h"
#include "XYZ/Renderer/Renderer.h"


#include <glm/gtx/transform.hpp>

namespace XYZ {
	static IGContext* s_Context;
	static size_t s_PoolHandle = 0;
	static glm::vec2 s_MousePosition = glm::vec2(0.0f);


	void IG::Init()
	{
		s_Context = new IGContext();
	}

	void IG::Shutdown()
	{
		delete s_Context;
	}

	void IG::BeginFrame(const glm::mat4& viewProjectionMatrix)
	{
		XYZ_ASSERT(s_Context, "InGuiContext is not initialized");
		glm::mat4 viewMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f));
		viewMatrix = glm::inverse(viewMatrix);


		if (s_Context->RenderData.Rebuild)
		{
			s_Context->Mesh.Quads.clear();
			s_Context->Mesh.Lines.clear();
			s_Context->RenderData.RebuildMesh(s_Context->Allocator, s_Context->Mesh);
		}
		Renderer2D::BeginScene(viewProjectionMatrix * viewMatrix);	
	}

	void IG::EndFrame()
	{
		Renderer2D::SetMaterial(s_Context->RenderData.Material);
		for (auto& it : s_Context->Mesh.Quads)
		{
			Renderer2D::SubmitQuadNotCentered(it.Position, it.Size, it.TexCoord, it.TextureID, it.Color);
		}
		for (auto& it : s_Context->Mesh.Lines)
		{
			Renderer2D::SubmitLine(it.P0, it.P1, it.Color);
		}

		Renderer2D::Flush();
		Renderer2D::FlushLines();
		Renderer2D::EndScene();
		Renderer::WaitAndRender();
		s_Context->RenderData.Rebuild = false;
	}

	void IG::OnEvent(Event& event)
	{
		if (event.GetEventType() == EventType::MouseButtonPressed)
		{
			s_Context->Input.OnMouseButtonPress((MouseButtonPressEvent&)event, *s_Context);
		}
		else if (event.GetEventType() == EventType::MouseButtonReleased)
		{
			s_Context->Input.OnMouseButtonRelease((MouseButtonReleaseEvent&)event, *s_Context);
		}
		else if (event.GetEventType() == EventType::MouseMoved)
		{
			s_Context->Input.OnMouseMove((MouseMovedEvent&)event, *s_Context);
		}
		if (event.Handled)
			s_Context->RenderData.Rebuild = true;
	}

	void IG::BeginUI(size_t handle)
	{
		s_PoolHandle = handle;
	}

	void IG::EndUI()
	{
	}

	void IG::Separator()
	{

	}

	std::pair<size_t, size_t> IG::AllocateUI(const std::initializer_list<IGHierarchyElement>& hierarchy, size_t** handles)
	{
		return s_Context->Allocator.CreatePool(hierarchy, handles);
	}

	void IG::End(size_t handle)
	{

	}


	template <>
	IGReturnType IG::UI<IGWindow>(size_t handle, const char* label)
	{
		IGWindow* window = s_Context->Allocator.Get<IGWindow>(s_PoolHandle, handle);
		return window->getAndRestartReturnType();
	}

	template <>
	IGReturnType IG::UI<IGCheckbox>(size_t handle, const char* label, bool& checked)
	{	
		IGCheckbox* checkbox = s_Context->Allocator.Get<IGCheckbox>(s_PoolHandle, handle);
		return checkbox->getAndRestartReturnType();
	}

	IGContext& IG::getContext()
	{
		return *s_Context;
	}
}