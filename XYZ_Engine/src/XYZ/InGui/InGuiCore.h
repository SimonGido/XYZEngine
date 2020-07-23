#pragma once
#include "XYZ/Gui/Font.h"

#include "XYZ/Renderer/Material.h"
#include "XYZ/Renderer/SubTexture2D.h"

#include <glm/glm.hpp>

namespace XYZ {
	namespace InGui {
		struct InGuiWindow
		{
			glm::vec2 Position;
			glm::vec2 Size;
			bool Collapsed;
			bool Modified = false;
		};

		struct InGuiRenderData
		{
			glm::vec4 HooverColor;
			glm::vec4 TextColor;
			uint32_t TextureID;
			uint32_t FontTextureID;
			Ref<Material> Material;
			Ref<Font> Font;
			Ref<SubTexture2D> ButtonSubTexture;
			Ref<SubTexture2D> CheckboxSubTextureChecked;
			Ref<SubTexture2D> CheckboxSubTextureUnChecked;
			Ref<SubTexture2D> SliderSubTexture;
			Ref<SubTexture2D> SliderHandleSubTexture;
			Ref<SubTexture2D> WindowSubTexture;
			Ref<SubTexture2D> MinimizeButtonSubTexture;
		};

		struct InGuiFrameData
		{
			InGuiWindow* CurrentWindow;
			uint32_t WindowSizeX;
			uint32_t WindowSizeY;

			glm::vec2 ModifiedWindowMouseOffset;
			glm::vec2 WindowSpaceOffset;
			glm::vec2 MousePosition;

			float MaxHeightInRow;

			uint8_t IsResizing;

			// Todo swap for bitset
			bool LeftMouseButtonDown;
			bool RightMouseButtonDown;
			bool IsWindowModified;

			bool ActiveWidget;
			bool ActiveWindow;

		};

		struct InGuiContext
		{
			InGuiFrameData InGuiData;
			InGuiRenderData InGuiRenderData;
			std::unordered_map<std::string, bool> CacheStates;
			std::unordered_map<std::string, InGuiWindow> InGuiWindows;
		};


	}
}