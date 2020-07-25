#pragma once
#include "InGuiCore.h"

#include <glm/glm.hpp>

namespace XYZ {
	// Instant Gui
	namespace InGui {

		extern InGuiContext* g_InContext;
				
		
		bool Begin(const std::string & name, const glm::vec2 & position, const glm::vec2 & size);
		void End();

		bool Button(const std::string & name, const glm::vec2 & size);
		bool Checkbox(const std::string & name, const glm::vec2 & size);
		bool Slider(const std::string & name, const glm::vec2 & size, float& value, float valueScale = 1.0f);
		bool Image(const std::string & name, uint32_t rendererID, const glm::vec2 & size);

		bool Text(const std::string& text, const glm::vec2& scale);
		bool ColorPicker4(const std::string& name,const glm::vec2& size,const glm::vec4& pallete, glm::vec4& color);
		bool ColorPallete4(const std::string& name, const glm::vec2& size, glm::vec4& color);

		bool RenderWindow(const std::string & name, uint32_t rendererID, const glm::vec2 & position, const glm::vec2 & size, float panelSize);
		
		void Separator();
	}
}