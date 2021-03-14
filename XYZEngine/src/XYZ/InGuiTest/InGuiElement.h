#pragma once
#include "XYZ/Renderer/SubTexture.h"

#include <glm/glm.hpp>

namespace XYZ {
	enum class IGTextCenter : uint8_t
	{
		Left,
		Right,
		Middle,
		None
	};

	struct IGLayout
	{
		float LeftPadding = 10.0f, RightPadding = 10.0f, TopPadding = 10.0f, BottomPadding = 10.0f;
		float SpacingX = 5.0f;
		float SpacingY = 5.0f;
	};

	struct IGStyle
	{
		IGTextCenter LabelCenter = IGTextCenter::Middle;
		IGLayout Layout;
		bool AutoPosition = true;
		bool NewRow = true;
		bool RenderFrame = false;
	};

	enum class IGElementType
	{
		Window,
		ImageWindow,
		Button,
		Checkbox,
		Slider,
		None
	};

	enum class IGReturnType
	{
		None,
		Hoovered,
		Clicked,
		Released,
	};

	class IGElement
	{
	public:
		IGElement(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);

		virtual bool OnLeftClick(const glm::vec2& mousePosition) {};
		virtual bool OnMouseHoover(const glm::vec2& mousePosition) {};

		glm::vec2  AbsolutePosition;
		glm::vec2  Position;
		glm::vec2  Size;
		glm::vec4  Color;	
		glm::vec4  FrameColor;
	
		IGStyle    Style;
		IGElement* Parent = nullptr;
		IGReturnType ReturnType = IGReturnType::None;
	};

	class IGWindow : public IGElement
	{
	public:
		IGWindow(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
		
		virtual bool OnLeftClick(const glm::vec2& mousePosition) override;
		
		enum Flags
		{
			Initialized = BIT(0),
			Hoovered = BIT(1),
			Collapsed = BIT(2),
			Docked = BIT(3)
		};

		uint8_t Flags = 0;

		static constexpr float PanelHeight = 25.0f;
	};
	class IGImageWindow : public IGElement
	{
	};
	class IGButton : public IGElement
	{
	};
	class IGCheckbox : public IGElement
	{
	};
	class IGSlider : public IGElement
	{
	};
}