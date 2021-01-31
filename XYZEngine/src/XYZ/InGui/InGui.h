#pragma once

#include "XYZ/Event/Event.h"
#include "XYZ/Event/InputEvent.h"

#include "InGuiStructures.h"

#include <glm/glm.hpp>

namespace XYZ {

	
	class InGui
	{
	public:
		static void Init();
		static void Destroy();

		static void BeginFrame(const glm::mat4& viewProjectionMatrix);
		static void EndFrame();
		static void OnEvent(Event& event);
		static void SetLayout(uint32_t id, const InGuiLayout& layout);
		static void Separator();
		
		static void SetWindowFlags(uint32_t id, uint16_t flags);
		static void SetPosition(const glm::vec2& position);
		static uint16_t GetWindowFlags(uint32_t id);
		static glm::vec2 GetPosition();

		static bool Begin(uint32_t id, const char* name, const glm::vec2& position, const glm::vec2& size);
		static bool ImageWindow(uint32_t id, const char* name, const glm::vec2& position, const glm::vec2& size, Ref<SubTexture> subTexture);
		static void End();
		static bool BeginGroup(const char* name, bool& open);

		static uint8_t PushNode(const char* name, const glm::vec2& size, bool& open, bool highlight = false);
		static uint8_t Dropdown(const char* name, const glm::vec2& size, bool& open);
		static void BeginChildren();
		static void EndChildren();


		static uint8_t Button(const char* name, const glm::vec2& size);
		static uint8_t Checkbox(const char* name, const glm::vec2& size, bool& val);
		static uint8_t Slider(const char* name, const glm::vec2& size, float& val);
		static uint8_t Text(const char* text);
		static uint8_t Float(const char* name, const glm::vec2& size, float& val);
		static uint8_t Image(const glm::vec2& size, Ref<SubTexture> subTexture);

	private:
		static void saveLayout();
		static void loadLayout();
		static bool eraseOutOfBorders(size_t oldQuadCount, const glm::vec2& genSize, const InGuiWindow& window, InGuiMesh& mesh);
		static InGuiWindow& getInitializedWindow(uint32_t id, const glm::vec2& position, const glm::vec2& size);
		static void handleWindowMove();
		static void handleWindowResize();

		static bool onMouseButtonPress(MouseButtonPressEvent& event);
		static bool onMouseButtonRelease(MouseButtonReleaseEvent& event);
		static bool onMouseMove(MouseMovedEvent& event);
		static bool onKeyTyped(KeyTypedEvent& event);
		static bool onKeyPressed(KeyPressedEvent& event);

	private:
		static InGuiContext* s_Context;
	};

}