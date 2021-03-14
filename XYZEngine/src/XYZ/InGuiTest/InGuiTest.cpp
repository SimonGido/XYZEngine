#include "stdafx.h"
#include "InGuiTest.h"


#include "InGuiContext.h"

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

	void IG::BeginFrame()
	{
	}

	void IG::EndFrame()
	{
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

		s_Parent = checkbox;
		return checkbox->ReturnType;
	}
}