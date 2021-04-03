#pragma once
#include "IGRenderer.h"

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
		ImageWindow,
		Group,
		Pack,
		Separator,
		Button,
		Checkbox,
		Slider,
		Text,
		Float,
		Int,
		String,
		Tree,
		Dropdown,
		Scrollbox,
		Image,
		None
	};

	enum class IGReturnType
	{
		None,
		Hoovered,
		Clicked,
		Released,
		Modified,
	};

	class IGPool;
	class IGElement
	{
	public:
		IGElement(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, IGElementType type);
		IGElement(IGElement&& other) noexcept;
		virtual ~IGElement() = default;

		virtual bool OnLeftClick(const glm::vec2& mousePosition, bool& handled) { return false; };
		virtual bool OnLeftRelease(const glm::vec2& mousePosition, bool& handled) { return false; }
		virtual bool OnMouseMove(const glm::vec2& mousePosition, bool& handled) { return false; };
		virtual bool OnMouseScroll(const glm::vec2& mousePosition, float offset, bool& handled) { return false; }
		virtual bool OnKeyType(char character, bool& handled) { return false; }

		virtual bool OnKeyPress(int32_t mode, int32_t key, bool& handled) { return false; }
		virtual glm::vec2 GenerateQuads(IGMesh& mesh, IGRenderData& renderData, uint32_t scissorIndex = 0) { return glm::vec2(0.0f); };
		virtual glm::vec2 BuildMesh(IGElement* root, IGRenderData& renderData, IGPool& pool, IGMesh& mesh, uint32_t scissorIndex = 0);

		IGElement* FindRoot();

		glm::vec2	Position;
		glm::vec2	Size;
		glm::vec4	Color;	
		glm::vec4	FrameColor;
		std::string Label;

		IGStyle				Style;
		IGElement*			Parent = nullptr;
		bool				Active = true;
		bool				ActiveChildren = true;
		bool				ListenToInput = true;
		const IGElementType	ElementType = IGElementType::None;

		glm::vec2 GetAbsolutePosition() const;
		int32_t GetID() const { return ID; }
	private:
		IGReturnType getAndRestartReturnType();


	protected:
		IGReturnType  ReturnType = IGReturnType::None;
		
		int32_t		  ID = -1;


		friend class IG;
		friend class IGPool;
		friend class IGSerializer;
	};
}