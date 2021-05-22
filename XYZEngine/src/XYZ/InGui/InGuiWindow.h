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
		glm::vec2	  GroupSize;
	};

	enum class AxisPlacement
	{
		Vertical, Horizontal
	};

	namespace InGuiWindowEditFlags {
		enum Flags
		{
			Hoovered = BIT(0),
			Collapsed = BIT(1),
			BlockEvents = BIT(2),
			Moving = BIT(3),
			ResizeLeft = BIT(4),
			ResizeRight = BIT(5),
			ResizeBottom = BIT(6)
		};
	}
	namespace InGuiWindowStyleFlags {
		enum Flags
		{
			ScrollEnabled	= BIT(0),
			PanelEnabled	= BIT(1),
			MenuEnabled		= BIT(2),
			DockingEnabled  = BIT(3),
			FrameEnabled	= BIT(4)
		};
	}

	struct InGuiWindow
	{
		InGuiWindow();
		
		void			 PushItselfToDrawlist(const glm::vec4& color, InGuiClipID clipID = 0);
		void			 PushTextClipped(const char* text, const glm::vec4& color, const glm::vec2& posMin, const glm::vec2& posMax, const glm::vec2* textSize = nullptr);
		void		     PushTextNotClipped(const char* text, const glm::vec4& color, const glm::vec2& posMin, const glm::vec2& posMax, const glm::vec2* textSize = nullptr);
		void		     PushText(const char* text, const glm::vec4& color, const glm::vec2& posMin, const glm::vec2& posMax, const glm::vec2* textSize = nullptr);
		void			 PushQuad(const glm::vec4& color, const glm::vec4& texCoord, const glm::vec2& pos, const glm::vec2& size, uint32_t textureID = InGuiConfig::sc_DefaultTexture);
		void			 PushQuadNotClipped(const glm::vec4& color, const glm::vec4& texCoord, const glm::vec2& pos, const glm::vec2& size, uint32_t textureID = InGuiConfig::sc_DefaultTexture);
		void			 PushLine(const glm::vec2& p0, const glm::vec2& p1, const glm::vec4& color);

		void			 PushTextClippedOverlay(const char* text, const glm::vec4& color, const glm::vec2& posMin, const glm::vec2& posMax, const glm::vec2* textSize = nullptr);
		void		     PushTextNotClippedOverlay(const char* text, const glm::vec4& color, const glm::vec2& posMin, const glm::vec2& posMax, const glm::vec2* textSize = nullptr);
		void		     PushTextOverlay(const char* text, const glm::vec4& color, const glm::vec2& posMin, const glm::vec2& posMax, const glm::vec2* textSize = nullptr);
		void			 PushQuadOverlay(const glm::vec4& color, const glm::vec4& texCoord, const glm::vec2& pos, const glm::vec2& size, uint32_t textureID = InGuiConfig::sc_DefaultTexture);
		void			 PushQuadNotClippedOverlay(const glm::vec4& color, const glm::vec4& texCoord, const glm::vec2& pos, const glm::vec2& size, uint32_t textureID = InGuiConfig::sc_DefaultTexture);
		void			 SetParent(InGuiWindow* parent);

		bool			 NextWidgetClipped(const glm::vec2& size);
		glm::vec2		 MoveCursorPosition(const glm::vec2& size);
		void			 SetCursorPosition(const glm::vec2& position);
		void			 EndGroup();

		bool			 ResolveResizeFlags(const glm::vec2& mousePosition, bool checkCollision = false);
		bool			 HandleResize(const glm::vec2& mousePosition);
		bool			 HandleHoover(const glm::vec2& mousePosition);
		bool			 HandleMove(const glm::vec2& mousePosition, const glm::vec2& offset);
		void			 HandleScrollbars();
		bool			 IsFocused() const;
		bool			 IsChild() const;
		InGuiID			 GetID(const char* name) const;
		glm::vec2		 GetRealSize() const;

		InGuiRect		 Rect() const;
		InGuiRect        RealRect() const;
		InGuiRect		 PanelRect() const;
		InGuiRect		 MinimizeRect() const;
		InGuiRect		 ClipRect() const;
		InGuiRect		 ClipRect(uint32_t viewportHeight) const;

		std::string			 Name;
		InGuiClipID			 ClipID;
		InGuiWindowFlags	 StyleFlags;
		InGuiWindowFlags	 EditFlags;
		InGuiWindowFrameData FrameData;
		AxisPlacement		 Axis;

		glm::vec2			 Position;
		glm::vec2			 Size;
		glm::vec2			 Scroll;


		bool				 IsActive;
		bool				 ScrollBarX;
		bool				 ScrollBarY;
		bool				 Grouping;
		glm::vec2			 ScrollBarSize;
		InGuiDrawList*		 DrawListInUse;
		InGuiDrawList		 DrawList;
		InGuiWindow*		 Parent;

		static constexpr float sc_ResizeThresholdX = 5.0f;
		static constexpr float sc_ResizeThresholdY = 5.0f;
	};
}