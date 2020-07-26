#pragma once
#include "XYZ/Gui/Font.h"

#include "XYZ/Renderer/Material.h"
#include "XYZ/Renderer/SubTexture2D.h"
#include "XYZ/Renderer/Mesh.h"

#include <glm/glm.hpp>

namespace XYZ {
	namespace InGui {

		enum InGuiWindowFlags
		{
			Collapsed = 1 << 0,
			Moved	  = 1 << 1,
			Resized	  = 1 << 2,
			Hoovered  = 1 << 3,
			Docked	  = 1 << 4
		};

		enum InGuiFrameDataFlags
		{
			WindowRightResize    = 1 << 0,
			WindowLeftResize     = 1 << 1,
			WindowBottomResize   = 1 << 2,
			WindowTopResize      = 1 << 3,
			LeftMouseButtonDown  = 1 << 4,
			RightMouseButtonDown = 1 << 5,
			ClickHandled         = 1 << 6
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
			uint32_t TextureID;
			uint32_t FontTextureID;
			uint32_t ColorPickerTextureID;
			Ref<Material> Material;
			Ref<Texture2D> GuiTexture;
			Ref<Texture2D> ColorPickerTexture;
			Ref<Font> Font;
			Ref<SubTexture2D> ButtonSubTexture;
			Ref<SubTexture2D> CheckboxSubTextureChecked;
			Ref<SubTexture2D> CheckboxSubTextureUnChecked;
			Ref<SubTexture2D> SliderSubTexture;
			Ref<SubTexture2D> SliderHandleSubTexture;
			Ref<SubTexture2D> WindowSubTexture;
			Ref<SubTexture2D> MinimizeButtonSubTexture;
			Ref<SubTexture2D> DockSpaceSubTexture;
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

			uint8_t Flags;		
		};

		struct InGuiConfig
		{
			glm::vec4 DefaultColor;
			glm::vec4 HooverColor;

			glm::vec2 NameScale;
			float MaxTextLength;
		};

		struct InGuiContext
		{
			InGuiFrameData InGuiData;
			InGuiRenderData InGuiRenderData;
			InGuiConfig InGuiConfig;

			std::unordered_map<std::string, bool> CacheStates;
			std::unordered_map<std::string, InGuiWindow> InGuiWindows;
		};


		void Init(const InGuiRenderData& renderData,const InGuiConfig& config);
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