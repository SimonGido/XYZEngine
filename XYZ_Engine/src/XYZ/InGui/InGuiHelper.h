#pragma once

#include "XYZ/Renderer/Mesh.h"
#include "XYZ/Gui/Font.h"
#include "InGuiCore.h"

#include <glm/glm.hpp>


namespace XYZ {
	namespace InGui {

		extern InGuiContext* g_InContext;

	
		glm::vec2 MouseToWorld(const glm::vec2& point);
		glm::vec2 StringToVec2(const std::string& src);
		glm::vec2 HandleWindowSpacing(const glm::vec2& uiSize);
		glm::vec4 CalculatePixelColor(const glm::vec4& pallete,const glm::vec2& position, const glm::vec2& size);
		glm::vec4 ColorFrom6SegmentColorRectangle(const glm::vec2& position, const glm::vec2& size);

		bool Collide(const glm::vec2& pos, const glm::vec2& size, const glm::vec2& point);
		bool DetectMove(const InGuiWindow& window);
		bool DetectResize(const InGuiWindow& window);
		bool DetectCollapse(const InGuiWindow& window);


		void HandleMouseInput(InGuiWindow& window);
		void HandleResize(InGuiWindow& window);
		void HandleMove(InGuiWindow& window);

		void Generate6SegmentColorRectangle(const glm::vec2& size, Vertex* buffer);
		void GenerateInGuiText(InGuiText& text, const Ref<Font>& font, const std::string& str, const glm::vec2& position, const glm::vec2& scale, float length, const glm::vec4& color = { 1,1,1,1 });
	
	
		
		void InsertWindowInDockSpace(InGuiWindow& window);
		void RemoveWindowFromDockSpace(InGuiWindow& window);

		void SplitDockNode(InGuiDockNode* node, uint8_t axis);
		void UnSplitDockNode(InGuiDockNode* node);
	}
}