#pragma once
#include "InGuiStructures.h"


namespace XYZ {

	class InGuiFactory
	{
	public:
		static void GenerateWindow(InGuiWindow& window, const InGuiRenderConfiguration& renderConfig);
		static void GenerateRenderWindow(InGuiWindow& window,uint32_t rendererID, InGuiRenderConfiguration& renderConfig);
		static void GenerateButton(const glm::vec2& position, const glm::vec2& size,const glm::vec4& color,const std::string& name,InGuiPerFrameData& frameData, const InGuiRenderConfiguration& renderConfig);
		static void GenerateCheckbox(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, const std::string& name, bool value, InGuiPerFrameData& frameData, const InGuiRenderConfiguration& renderConfig);
		static void GenerateSlider(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, const std::string& name,float value, InGuiPerFrameData& frameData, const InGuiRenderConfiguration& renderConfig);
	};
}