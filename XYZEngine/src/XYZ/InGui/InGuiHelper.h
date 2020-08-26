#pragma once

#include "XYZ/Renderer/InGuiRenderer.h"
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
		bool CollideSquares(const glm::vec2& pos, const glm::vec2& size, const glm::vec2& otherPos, const glm::vec2& otherSize);
		bool DetectMove(const InGuiWindow& window);
		bool DetectResize(const InGuiWindow& window);
		bool DetectCollapse(const InGuiWindow& window);


		void HandleMouseInput(InGuiWindow* window);
		void HandleMove(InGuiWindow* window);
		void HandleResize(InGuiWindow* window);
		void HandleDocking(InGuiWindow* window);
		void HandleActivity(InGuiWindow* window);

		void Generate6SegmentColorRectangle(InGuiMesh& mesh,const glm::vec2& position, const glm::vec2& size, uint32_t textureID);
		void GenerateInGuiQuad(InGuiMesh& mesh, InGuiVertex* vertices, size_t count);
		void GenerateInGuiQuad(InGuiMesh& mesh, const glm::vec2& position, const glm::vec2& size,const glm::vec4& texCoord, uint32_t textureID, const glm::vec4& color = { 1,1,1,1 });
		void GenerateInGuiImage(InGuiMesh& mesh,uint32_t rendererID, const glm::vec2& position, const glm::vec2& size, const glm::vec4& texCoord = { 0,0,1,1 }, const glm::vec4& color = { 1,1,1,1 });

		std::pair<int32_t, int32_t> GenerateInGuiText(InGuiMesh& mesh, const Ref<Font>& font, const std::string& str, const glm::vec2& position, const glm::vec2& scale, float length, uint32_t textureID, const glm::vec4& color = { 1,1,1,1 });
		std::pair<int32_t, int32_t> GenerateInGuiText(InGuiVertex* vertices, const Ref<Font>& font, const std::string& str, const glm::vec2& position, const glm::vec2& scale, float length, uint32_t textureID, const glm::vec4& color = { 1,1,1,1 });
		void MoveVertices(InGuiVertex* vertices, const glm::vec2& position, size_t offset, size_t count);
		
		void HandleInputText(std::string& text);
		void HandleInputNumber(std::string& text);
	}
}