#pragma once
#include "XYZ/Gui/Font.h"

#include "XYZ/Renderer/Material.h"
#include "XYZ/Renderer/SubTexture2D.h"

#include <glm/glm.hpp>

namespace XYZ {

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
		uint32_t WindowSizeX, WindowSizeY;
		glm::vec2 MousePosition;
		glm::vec2 CurrentPanelPosition;
		bool LeftMouseButtonDown;
		bool RightMouseButtonDown;
		bool ActiveWidget;
		bool ActivePanel;
		bool ColapsedPanel;
		int32_t ModifiedPanel;
	};
	// Instant Gui
	class InGui
	{
	public:
		static void Init(const InGuiRenderData& renderData);
		static void Shutdown();

		static void BeginFrame();
		static void EndFrame();

		static bool Begin(const std::string& name,const glm::vec2& position,const glm::vec2& size, float panelSize);
		static void End();
		static bool Button(const std::string& name, const glm::vec2& position, const glm::vec2& size);
		static bool Checkbox(const std::string& name, const glm::vec2& position, const glm::vec2& size);
		static bool Slider(const std::string& name, const glm::vec2& position, const glm::vec2& size, float& value);
		static bool Image(const std::string& name, uint32_t rendererID,const glm::vec2& position, const glm::vec2& size);

		static bool RenderWindow(const std::string& name, uint32_t rendererID, const glm::vec2& position, const glm::vec2& size, float panelSize);

		static InGuiFrameData& GetData();
	};
}