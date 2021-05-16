#pragma once
#include "InGuiDrawList.h"
#include "InGuiConfig.h"

#include <glm/glm.hpp>


namespace XYZ {
	
	
	using InGuiWindowFlags = uint32_t;
	using InGuiWindowID = uint32_t;

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
		
		void			 PushItselfToDrawlist(const glm::vec4& color, const InGuiConfig& config);
		bool			 ResolveResizeFlags(const glm::vec2& mousePosition, bool checkCollision = false);
		bool			 HandleResize(const glm::vec2& mousePosition);
		bool			 HandleHoover(const glm::vec2& mousePosition);

		InGuiRect		 Rect() const { return InGuiRect(Position, Position + Size); }
		InGuiRect		 PanelRect() const { return InGuiRect(Position, Position + glm::vec2(Size.x, PanelHeight)); }
		InGuiRect		 MinimizeRect() const;
		InGuiRect		 ClipRect() const;
		InGuiRect		 ClipRect(uint32_t viewportHeight) const;

		std::string		 Name;
		InGuiWindowID	 ID;
		InGuiWindowFlags Flags;

		glm::vec2		 Position;
		glm::vec2		 Size;
		glm::vec2		 Scroll;

		float			 PanelHeight;
		float			 LabelOffset;
		
		InGuiDrawList	 DrawList;


		static constexpr float sc_ResizeThresholdX = 5.0f;
		static constexpr float sc_ResizeThresholdY = 5.0f;
	};
}