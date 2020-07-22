#pragma once
#include "InGuiCore.h"

#include <glm/glm.hpp>

namespace XYZ {
	// Instant Gui
	namespace InGui {

		extern InGuiContext* g_InContext;
				
		void Init(const InGuiRenderData & renderData);
		void Shutdown();

		void BeginFrame();
		void EndFrame();

		bool Begin(const std::string & name, const glm::vec2 & position, const glm::vec2 & size, float panelSize);
		void End();
		bool Button(const std::string & name, const glm::vec2 & size);
		bool Checkbox(const std::string & name, const glm::vec2 & size);
		bool Slider(const std::string & name, const glm::vec2 & size, float& value);
		bool Image(const std::string & name, uint32_t rendererID, const glm::vec2 & size);

		bool RenderWindow(const std::string & name, uint32_t rendererID, const glm::vec2 & position, const glm::vec2 & size, float panelSize);


		InGuiFrameData& GetData();
	}
}