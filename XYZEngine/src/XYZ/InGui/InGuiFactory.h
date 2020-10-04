#pragma once
#include "InGuiStructures.h"


namespace XYZ {

	class InGuiFactory
	{
	public:
		static void GenerateWindow(const char* name, InGuiWindow& window, const InGuiRenderConfiguration& renderConfig);
		static void GenerateRenderWindow(const char* name,InGuiWindow& window,uint32_t rendererID, InGuiPerFrameData& frameData, const InGuiRenderConfiguration& renderConfig);
		static void GenerateButton(const glm::vec2& position, const glm::vec2& size,const glm::vec4& color, const char* name,InGuiMesh& mesh, const InGuiRenderConfiguration& renderConfig);
		static void GenerateCheckbox(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, const char* name, bool value, InGuiMesh& mesh, const InGuiRenderConfiguration& renderConfig);
		static void GenerateSlider(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, const char* name,float value,glm::vec2& windowSpaceOffset, InGuiMesh& mesh, const InGuiRenderConfiguration& renderConfig);
		static void GenerateImage(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, uint32_t rendererID, InGuiMesh& mesh, std::vector<TextureRendererIDPair>& texturePairs, const InGuiRenderConfiguration& renderConfig, float tilingFactor);
		static void GenerateTextArea(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, const char* name, const char* text, glm::vec2& windowSpaceOffset, InGuiMesh& mesh, const InGuiRenderConfiguration& renderConfig);
		static TextInfo GenerateText(const glm::vec2& scale, const glm::vec4& color, const char* text, InGuiMesh& mesh, const InGuiRenderConfiguration& renderConfig);
		static void GenerateColorPicker4(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, InGuiMesh& mesh, const InGuiRenderConfiguration& renderConfig);
		static void Generate6SegmentColorRectangle(const glm::vec2& position, const glm::vec2& size, InGuiMesh& mesh, const InGuiRenderConfiguration& renderConfig);
		static void GenerateGroup(const glm::vec2& position,const glm::vec4& color, const char* name, bool open, InGuiPerFrameData& frameData, const InGuiRenderConfiguration& renderConfig);
		static void GenerateQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, InGuiMesh& mesh, const InGuiRenderConfiguration& renderConfig);
		static void GenerateMenuBar(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, const char* name, InGuiPerFrameData& frameData, const InGuiRenderConfiguration& renderConfig);
		static void GenerateFrame(InGuiLineMesh& mesh, const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
		static void GenerateArrowLine(InGuiMesh& mesh,InGuiLineMesh& lineMesh, const glm::vec2& p0, const glm::vec2& p1,const glm::vec2& size, const InGuiRenderConfiguration& renderConfig);
		
		static void GenerateNode(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, const glm::vec4& panelColor, const char* name, InGuiMesh& mesh, const InGuiRenderConfiguration& renderConfig);
	};
}