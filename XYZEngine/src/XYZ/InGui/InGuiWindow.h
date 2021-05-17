#pragma once
#include "InGuiDrawList.h"
#include "InGuiConfig.h"

#include <glm/glm.hpp>


namespace XYZ {
	
	
	using InGuiWindowFlags = uint32_t;
	using InGuiClipID = uint32_t;
	using InGuiID = size_t;

	struct InGuiWindow;
	struct InGuiWindowFrameData
	{
		InGuiWindowFrameData(InGuiWindow* window);
	
		union Parameters
		{
			float RowHeight;
			float RowWidth;
		};
		Parameters	  Params;
		glm::vec2	  CursorPos;
		glm::vec2	  ScrollMax;
	};

	enum class AxisPlacement
	{
		Vertical, Horizontal
	};
	struct InGuiWindow
	{
		InGuiWindow();

		enum Flags
		{
			Hoovered	 = BIT(0),
			Collapsed	 = BIT(1),
			BlockEvents  = BIT(2),
			Moving		 = BIT(3),
			ResizeLeft	 = BIT(4),
			ResizeRight  = BIT(5),
			ResizeBottom = BIT(6)
		};
		enum StyleFlags
		{
			ScrollEnabled = BIT(0),
			PanelEnabled  = BIT(1)
		};
		void			 PushItselfToDrawlist(const glm::vec4& color, const InGuiConfig& config);
		void			 PushTextClipped(const char* text, const glm::vec4& color, const glm::vec2& posMin, const glm::vec2& posMax, const glm::vec2* textSize = nullptr);
		void		     PushText(const char* text, const glm::vec4& color, const glm::vec2& posMin, const glm::vec2& posMax, const glm::vec2* textSize = nullptr);
		void			 PushQuad(const glm::vec4& color, const glm::vec4& texCoord, const glm::vec2& pos, const glm::vec2& size, uint32_t textureID = InGuiConfig::sc_DefaultTexture);

		bool			 NextWidgetClipped(const glm::vec2& size);
		glm::vec2		 MoveCursorPosition(const glm::vec2& size);
		void			 SetCursorPosition(const glm::vec2& position);

		bool			 ResolveResizeFlags(const glm::vec2& mousePosition, bool checkCollision = false);
		bool			 HandleResize(const glm::vec2& mousePosition);
		bool			 HandleHoover(const glm::vec2& mousePosition);
		bool			 HandleMove(const glm::vec2& mousePosition, const glm::vec2& offset);
		void			 HandleScrollbars();
		bool			 IsFocused() const;

		InGuiRect		 Rect() const { return InGuiRect(Position, Position + Size); }
		InGuiRect		 PanelRect() const { return InGuiRect(Position, Position + glm::vec2(Size.x, PanelHeight)); }
		InGuiRect		 MinimizeRect() const;
		InGuiRect		 ClipRect() const;
		InGuiRect		 ClipRect(uint32_t viewportHeight) const;

		std::string			 Name;
		InGuiClipID			 ClipID;
		InGuiWindowFlags	 StyleFlags;
		InGuiWindowFlags	 Flags;
		InGuiWindowFrameData FrameData;
		AxisPlacement		 Axis;

		glm::vec2			 Position;
		glm::vec2			 Size;
		glm::vec2			 Scroll;

		float				 PanelHeight;
		float				 LabelOffset;
		bool				 IsActive;
		bool				 ScrollBarX;
		bool				 ScrollBarY;
		glm::vec2			 ScrollBarSize;
		InGuiDrawList		 DrawList;


		static constexpr float sc_ResizeThresholdX = 5.0f;
		static constexpr float sc_ResizeThresholdY = 5.0f;
	};
}