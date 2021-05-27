#pragma once
#include "InGuiDrawList.h"
#include "InGuiConfig.h"

#include <glm/glm.hpp>


namespace XYZ {
	
	using InGuiWindowFlags = uint32_t;
	using InGuiClipID = uint32_t;
	using InGuiID = size_t;

	static constexpr InGuiID InGuiInvalidID = 0;

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
		float		  TabOffet;
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
			FrameEnabled	= BIT(4),
			TabEnabled		= BIT(5),
			LabelEnabled    = BIT(6)
		};
	}

	struct InGuiDockNode;
	struct InGuiWindow
	{
		InGuiWindow(uint32_t workClipId, uint32_t panelClipID);
		
		void			 PushItselfToDrawlist(bool highlight, InGuiClipID clipID = 0);
		void			 PushTextClipped(const char* text, const glm::vec4& color, const glm::vec2& posMin, const glm::vec2& posMax, const glm::vec2* textSize = nullptr);
		void		     PushTextNotClipped(const char* text, const glm::vec4& color, const glm::vec2& posMin, const glm::vec2& posMax, const glm::vec2* textSize = nullptr);
		void		     PushText(const char* text, const glm::vec4& color, const glm::vec2& posMin, const glm::vec2& posMax, const glm::vec2* textSize = nullptr);
		void			 PushQuad(const glm::vec4& color, const glm::vec4& texCoord, const glm::vec2& pos, const glm::vec2& size, uint32_t textureID = InGuiConfig::DefaultTextureIndex);
		void			 PushQuadNotClipped(const glm::vec4& color, const glm::vec4& texCoord, const glm::vec2& pos, const glm::vec2& size, uint32_t textureID = InGuiConfig::DefaultTextureIndex);
		void			 PushLine(const glm::vec2& p0, const glm::vec2& p1, const glm::vec4& color);

		void			 PushTextClippedOverlay(const char* text, const glm::vec4& color, const glm::vec2& posMin, const glm::vec2& posMax, const glm::vec2* textSize = nullptr);
		void		     PushTextNotClippedOverlay(const char* text, const glm::vec4& color, const glm::vec2& posMin, const glm::vec2& posMax, const glm::vec2* textSize = nullptr);
		void		     PushTextOverlay(const char* text, const glm::vec4& color, const glm::vec2& posMin, const glm::vec2& posMax, const glm::vec2* textSize = nullptr);
		void			 PushQuadOverlay(const glm::vec4& color, const glm::vec4& texCoord, const glm::vec2& pos, const glm::vec2& size, uint32_t textureID = InGuiConfig::DefaultTextureIndex);
		void			 PushQuadNotClippedOverlay(const glm::vec4& color, const glm::vec4& texCoord, const glm::vec2& pos, const glm::vec2& size, uint32_t textureID = InGuiConfig::DefaultTextureIndex);
		void			 SetParent(InGuiWindow* parent);

		bool			 NextWidgetClipped(const glm::vec2& size);
		bool			 NextTabClipped();
		glm::vec2		 MoveCursorPosition(const glm::vec2& size);
		float			 MoveTabPosition(float size);
		void			 SetCursorPosition(const glm::vec2& position);
		void			 EndGroup();

		bool			 ResolveResizeFlags(const glm::vec2& mousePosition, bool checkCollision = false);
		bool			 HandleResize(const glm::vec2& mousePosition);
		bool			 HandleHoover(const glm::vec2& mousePosition);
		bool			 HandleMove(const glm::vec2& mousePosition, const glm::vec2& offset);
		void			 HandleScrollbars();
		bool			 IsResizing() const;
		bool			 IsFocused() const;
		bool			 IsChild() const;
		bool			 IsParentFocused() const;
		InGuiID			 GetID(const char* name) const;
		glm::vec2		 GetRealSize() const;
		glm::vec2		 TreeNodeOffset() const;

		InGuiRect		 Rect() const;
		InGuiRect        RealRect() const;
		InGuiRect		 PanelRect() const;
		InGuiRect        PanelClipRect(uint32_t viewportHeight) const;
		InGuiRect		 MinimizeRect() const;
		InGuiRect		 ClipRect() const;
		InGuiRect		 ClipRect(uint32_t viewportHeight) const;

		std::string				  Name;
		InGuiClipID				  ClipID;      // Clip id used in draw list
		const InGuiClipID		  WorkClipID;  // Clip id of work area
		const InGuiClipID		  PanelClipID; // Clipd id of panel
		InGuiWindowFlags		  StyleFlags;
		InGuiWindowFlags		  EditFlags;
		InGuiWindowFrameData	  FrameData;
		AxisPlacement			  Axis;
		InGuiID					  TabID;
								  
		glm::vec2				  Position;
		glm::vec2				  Size;
		glm::vec2				  OriginalSize; // Size before being docked
		glm::vec2				  Scroll;

		bool					  IsActive;
		bool					  IsInitialized;
		bool					  ScrollBarX;
		bool					  ScrollBarY;
		bool					  Grouping;
		glm::vec2				  ScrollBarSize;
		InGuiDrawList*			  DrawListInUse;
		InGuiDrawList			  DrawList;
		InGuiWindow*			  Parent;
		std::vector<InGuiWindow*> ChildWindows;
		InGuiDockNode*			  DockNode;
	};
}