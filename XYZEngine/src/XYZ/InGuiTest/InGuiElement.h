#pragma once
#include "InGuiRenderer.h"

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

	class IGElement
	{
	public:
		IGElement(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
		virtual ~IGElement() = default;

		virtual bool OnLeftClick(const glm::vec2& mousePosition) { return false; };
		virtual bool OnLeftRelease(const glm::vec2& mousePosition) { return false; }
		virtual bool OnMouseMove(const glm::vec2& mousePosition) { return false; };
		virtual glm::vec2 GenerateQuads(IGMesh& mesh, IGRenderData& renderData) { return glm::vec2(0.0f); };


		glm::vec2	Position;
		glm::vec2	Size;
		glm::vec4	Color;	
		glm::vec4	FrameColor;
		std::string Label = "Test";

		IGStyle		Style;
		IGElement*	Parent = nullptr;
		bool		Active = false;


		glm::vec2 GetAbsolutePosition() const;
		IGReturnType GetAndRestartReturnType();
	protected:
		IGReturnType ReturnType = IGReturnType::None;
		IGElementType ElementType = IGElementType::None;

	};

	
}