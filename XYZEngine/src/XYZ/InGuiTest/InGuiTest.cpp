#include "stdafx.h"
#include "InGuiTest.h"

#include "InGuiContext.h"
#include "XYZ/Core/Input.h"
#include "XYZ/Renderer/Renderer2D.h"
#include "XYZ/Renderer/Renderer.h"


#include <glm/gtx/transform.hpp>

namespace XYZ {
	static IGContext* s_Context;

	static IGElement* s_Parent = nullptr;
	static size_t s_PoolHandle = 0;

	static float s_HighestInRow = 0.0f;
	static glm::vec2 s_AbsoluteOffset = glm::vec2(0.0f);
	static glm::vec2 s_MousePosition = glm::vec2(0.0f);

	namespace Helper {

		static void AbsolutePosition(IGElement* element, IGElement* parent)
		{
			element->AbsolutePosition = element->Position;
			if (parent)
			{
				element->AbsolutePosition += parent->AbsolutePosition;
			}
		}

		static bool ResolvePosition(size_t oldQuadCount, const glm::vec2& genSize, IGElement* element, IGElement* parent, IGMesh& mesh)
		{
			if (parent)
			{
				if (parent->Style.AutoPosition)
				{
					float xBorder = parent->AbsolutePosition.x + parent->Size.x - parent->Style.Layout.RightPadding;
					float yBorder = parent->AbsolutePosition.y + parent->Size.y - parent->Style.Layout.BottomPadding;
					if (s_AbsoluteOffset.x > genSize.x > xBorder)
					{
						if (!parent->Style.NewRow)
						{
							s_AbsoluteOffset.x += genSize.x + parent->Style.Layout.SpacingX;
							mesh.Quads.erase(mesh.Quads.begin() + oldQuadCount, mesh.Quads.end());
							return false;
						}
						else if (s_AbsoluteOffset.y + genSize.y > yBorder)
						{
							float oldX = mesh.Quads[oldQuadCount].Position.x;
							mesh.Quads[oldQuadCount].Position.x = parent->AbsolutePosition.x + parent->Style.Layout.LeftPadding;
							mesh.Quads[oldQuadCount].Position.y += s_HighestInRow + parent->Style.Layout.TopPadding;
							for (size_t i = oldQuadCount + 1; i < mesh.Quads.size(); ++i)
							{
								float diff = mesh.Quads[i].Position.x - oldX;
								oldX = mesh.Quads[i].Position.x;

								mesh.Quads[i].Position.x = mesh.Quads[i - 1].Position.x + diff;
								mesh.Quads[i].Position.y += s_HighestInRow + parent->Style.Layout.TopPadding;
							}
							IG::Separator();
							s_HighestInRow = genSize.y;
						}
						else
						{
							mesh.Quads.erase(mesh.Quads.begin() + oldQuadCount, mesh.Quads.end());
							return true;
						}
					}
				}
			}
			return true;
		}
	}

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
		
		for (auto& pool : s_Context->Allocator.GetPools())
		{
			for (size_t i = 0; i < pool.Size(); ++i)
				pool[i]->Active = false;
		}

		if (s_Context->RenderData.Rebuild)
		{
			s_Context->Mesh.Quads.clear();
			s_Context->Mesh.Lines.clear();
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
		XYZ_ASSERT(s_Parent, "No parent is active");
		s_AbsoluteOffset.x = s_Parent->Position.x + s_Parent->Style.Layout.LeftPadding;
		s_AbsoluteOffset.y += s_HighestInRow + s_Parent->Style.Layout.TopPadding;
		s_HighestInRow = 0.0f;
	}

	size_t IG::AllocateUI(const std::initializer_list<IGElementType>& types, size_t** handles)
	{
		return s_Context->Allocator.CreatePool(types, handles);
	}

	void IG::End(size_t handle)
	{
		IGElement* element = s_Context->Allocator.Get<IGElement>(s_PoolHandle, handle);
		s_Parent = element->Parent;
	}

	template <>
	IGReturnType IG::UI<IGElementType::Window>(size_t handle, const char* label)
	{
		IGWindow* window = s_Context->Allocator.Get<IGWindow>(s_PoolHandle, handle);
		window->Parent = s_Parent;
		window->Active = true;

		size_t oldQuadCount = s_Context->Mesh.Quads.size();
		Helper::AbsolutePosition(window, window->Parent);
		IGMeshFactoryData data = { IGRenderData::Window, window, &s_Context->Mesh, &s_Context->RenderData };
		glm::vec2 genSize = IGMeshFactory::GenerateUI<IGElementType::Window>(label, glm::vec4(1.0f), data);

		s_Parent = window;
		return window->ReturnType;
	}

	template <>
	IGReturnType IG::UI<IGElementType::Checkbox>(size_t handle, const char* label, bool& checked)
	{
		IGCheckbox* checkbox = s_Context->Allocator.Get<IGCheckbox>(s_PoolHandle, handle);
		checkbox->Parent = s_Parent;
		checkbox->Active = true;

		s_Parent = checkbox;
		return checkbox->ReturnType;
	}


	template<>
	uint8_t IG::GetFlags<IGElementType::Window>(size_t handle)
	{
		return s_Context->Allocator.Get<IGWindow>(s_PoolHandle, handle)->Flags;
	}
}