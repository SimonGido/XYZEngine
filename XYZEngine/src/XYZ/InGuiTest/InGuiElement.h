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
		IGTextCenter LabelCenter = IGTextCenter::Left;
		IGLayout Layout;
		bool AutoPosition = true;
		bool NewRow = true;
		bool RenderFrame = true;
	};

	enum class IGElementType
	{
		Window,
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

	class IGWindow;
	class IGElement
	{
	public:
		IGElement(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
		virtual ~IGElement() = default;

		virtual bool OnLeftClick(const glm::vec2& mousePosition) { return false; };
		virtual bool OnLeftRelease(const glm::vec2& mousePosition) { return false; }
		virtual bool OnMouseMove(const glm::vec2& mousePosition) { return false; };

		glm::vec2  AbsolutePosition;
		glm::vec2  Position;
		glm::vec2  Size;
		glm::vec4  Color;	
		glm::vec4  FrameColor;
	
		IGStyle		 Style;
		IGReturnType ReturnType = IGReturnType::None;
		IGElementType ElementType = IGElementType::None;

		bool		 Active = false;
	};

	class IGWindow : public IGElement
	{
	public:
		IGWindow(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
		
		virtual bool OnLeftClick(const glm::vec2& mousePosition) override;
		virtual bool OnLeftRelease(const glm::vec2& mousePosition) override;
		virtual bool OnMouseMove(const glm::vec2& mousePosition) override;

		enum Flags
		{
			Initialized = BIT(0),
			Hoovered	= BIT(1),
			Moved       = BIT(2),
			Collapsed   = BIT(3),
			Docked	    = BIT(4)
		};

		uint8_t Flags = 0;

		static constexpr float PanelHeight = 25.0f;
	};
	class IGButton : public IGElement
	{
	public:
		IGButton(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);


		IGWindow*	 Parent = nullptr;
	};
	class IGCheckbox : public IGElement
	{
	public:
		IGCheckbox(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);


		IGWindow*	 Parent = nullptr;
	};
	class IGSlider : public IGElement
	{
	public:
		IGSlider(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);


		IGWindow*	 Parent = nullptr;
	};
}