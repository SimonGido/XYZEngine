#pragma once
#include "XYZ/Gui/Font.h"

#include "XYZ/Renderer/Material.h"
#include "XYZ/Renderer/SubTexture2D.h"

#include <glm/glm.hpp>

namespace XYZ {
	namespace InGui {

		enum InGuiWindowFlags
		{
			Collapsed = 1 << 0,
			Moved	  = 1 << 1,
			Resized	  = 1 << 2,
			Hoovered  = 1 << 3
		};

		struct InGuiWindow
		{
			glm::vec2 Position;
			glm::vec2 Size;
			uint8_t Flags;
			static constexpr float PanelSize = 25.0f;
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

			bool ClickHandled;
		};

		struct InGuiContext
		{
			InGuiFrameData InGuiData;
			InGuiRenderData InGuiRenderData;
			std::unordered_map<std::string, bool> CacheStates;
			std::unordered_map<std::string, InGuiWindow> InGuiWindows;
		};


		void Init(const InGuiRenderData& renderData);
		void Shutdown();

		void BeginFrame();
		void EndFrame();


		void OnLeftMouseButtonRelease();
		void OnRightMouseButtonRelease();
		void OnLeftMouseButtonPress();
		void OnRightMouseButtonPress();

		InGuiFrameData& GetData();
	}
}