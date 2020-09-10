#pragma once
#include "InGuiStructures.h"


namespace XYZ {

	class InGuiFactory
	{
	public:
		static void GenerateWindow(InGuiWindow& window, const InGuiRenderConfiguration& renderConfig);
		static void GenerateRenderWindow(InGuiWindow& window,uint32_t rendererID, const InGuiRenderConfiguration& renderConfig);
		static void GenerateButton(const glm::vec2& position, const glm::vec2& size,const glm::vec4& color,const std::string& name,InGuiPerFrameData& frameData, const InGuiRenderConfiguration& renderConfig);
		static void GenerateCheckbox(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, const std::string& name, bool value, InGuiPerFrameData& frameData, const InGuiRenderConfiguration& renderConfig);
		static void GenerateSlider(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, const std::string& name,float value, InGuiPerFrameData& frameData, const InGuiRenderConfiguration& renderConfig);
		static void GenerateImage(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, uint32_t rendererID, InGuiPerFrameData& frameData, const InGuiRenderConfiguration& renderConfig);
		static void GenerateTextArea(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, const std::string& name, const std::string& text, InGuiPerFrameData& frameData, const InGuiRenderConfiguration& renderConfig);
		static std::pair<int32_t,int32_t> GenerateText(const glm::vec2& scale, const glm::vec4& color, const std::string& text, InGuiPerFrameData& frameData, const InGuiRenderConfiguration& renderConfig);
		static void GenerateColorPicker4(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, InGuiPerFrameData& frameData, const InGuiRenderConfiguration& renderConfig);
		static void Generate6SegmentColorRectangle(const glm::vec2& position, const glm::vec2& size, InGuiPerFrameData& frameData, const InGuiRenderConfiguration& renderConfig);
		static void GenerateGroup(const glm::vec2& position,const glm::vec4& color, const std::string& name, bool open, InGuiPerFrameData& frameData, const InGuiRenderConfiguration& renderConfig);
		static void GenerateQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, InGuiPerFrameData& frameData, const InGuiRenderConfiguration& renderConfig);
		static void GenerateMenuBar(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, const std::string& name, InGuiPerFrameData& frameData, const InGuiRenderConfiguration& renderConfig);
	};
}