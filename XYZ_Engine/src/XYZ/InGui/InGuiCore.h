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
	};

	struct InGuiFrameData
	{
		uint32_t WindowSizeX, WindowSizeY;
		glm::vec2 MousePosition;
		bool LeftMouseButtonDown;
		bool RightMouseButtonDown;
		bool ActiveWidget;
	};
	// Instant Gui
	class InGui
	{
	public:
		static void Init(const InGuiRenderData& renderData);
		static void BeginFrame();
		static void EndFrame();

		static bool Button(const std::string& name, const glm::vec2& position, const glm::vec2& size);
		static bool Checkbox(const std::string& name, const glm::vec2& position, const glm::vec2& size);
		static bool Slider(const std::string& name, const glm::vec2& position, const glm::vec2& size, float& value);
		static bool Image(const glm::vec2& position, const glm::vec2& size,);

		static InGuiFrameData& GetData();
	};
}