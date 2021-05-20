#pragma once
#include "InGuiContext.h"



namespace XYZ {

	class InGui
	{
	public:
		static void BeginFrame();
		static void EndFrame();

		static bool Begin(const char* name, InGuiWindowFlags flags = 0);
		static void End();
		static void Separator();

		enum Result { Hoover = BIT(0), Pressed = BIT(1) };

		static bool BeginMenuBar();
		static void EndMenuBar();

		static uint8_t BeginMenu(const char* label, float width);
		static void    EndMenu();
		static uint8_t MenuItem(const char* label);

		static uint8_t Button(const char* label, const glm::vec2& size);
		static uint8_t Checkbox(const char* label, const glm::vec2& size, bool& checked);
		static uint8_t SliderFloat(const char* label, const glm::vec2& size, float& value, float min, float max, const char* format = "%.3f");
		static uint8_t VSliderFloat(const char* label, const glm::vec2& size, float& value, float min, float max, const char* format = "%.3f");

		static uint8_t Float(const char* label, const glm::vec2& size, float& value, int decimalPrecision);

		static InGuiContext& GetContext();
	};
}