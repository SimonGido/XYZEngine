#pragma once

#include "XYZ/Renderer/Mesh.h"
#include "XYZ/Gui/Font.h"
#include "InGuiCore.h"

#include <glm/glm.hpp>


namespace XYZ {
	namespace InGui {

		extern InGuiContext* g_InContext;

		struct InGuiText
		{
			std::vector<Vertex> Vertices;
			int32_t Width = 0;
			int32_t Height = 0;
		};


		glm::vec2 MouseToWorld(const glm::vec2& point);
		glm::vec2 StringToVec2(const std::string& src);
		glm::vec2 HandleWindowSpacing(const glm::vec2& uiSize);

		bool Collide(const glm::vec2& pos, const glm::vec2& size, const glm::vec2& point);
		void DetectResize(const InGuiWindow& window);
		void HandleResize(InGuiWindow& window);

		void GenerateInGuiText(InGuiText& text, const Ref<Font>& font, const std::string& str, const glm::vec2& position, const glm::vec2& scale, float length, const glm::vec4& color = { 1,1,1,1 });
	}
}