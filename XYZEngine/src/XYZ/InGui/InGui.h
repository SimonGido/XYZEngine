#pragma once
#include "InGuiContext.h"



namespace XYZ {

	class InGui
	{
	public:
		static void Init(const std::string& filepath = std::string());
		static void Shutdown();

		static void BeginFrame();
		static void EndFrame();

		static InGuiID GetID(const char* name);
		static void FocusWindow(const char* name);

		static bool Begin(const char* name, InGuiWindowFlags flags = (
			 InGuiWindowStyleFlags::PanelEnabled 
		   | InGuiWindowStyleFlags::LabelEnabled
		));
		static void End();
		static void Separator();
		

		enum Result { Hoover = BIT(0), Pressed = BIT(1) };

		static void DockSpace();

		static bool BeginMenuBar();
		static void EndMenuBar();

		static void BeginGroup();
		static void EndGroup();

		static bool BeginTab(const char* label);
		static void EnableHighlight();
		static void DisableHighlight();

		static void    BeginTreeChild();
		static void    EndTreeChild();
		static uint8_t TreeNode(const char* label, const glm::vec2& size, bool& open);

		static uint8_t BeginMenu(const char* label, float width);
		static void    EndMenu();
		static uint8_t MenuItem(const char* label);

		static uint8_t Button(const char* label, const glm::vec2& size);
		static uint8_t Checkbox(const char* label, const glm::vec2& size, bool& checked);
		static uint8_t SliderFloat(const char* label, const glm::vec2& size, float& value, float min, float max, const char* format = "%.3f");
		static uint8_t VSliderFloat(const char* label, const glm::vec2& size, float& value, float min, float max, const char* format = "%.3f");

		static uint8_t Float(const char* label, const glm::vec2& size, float& value, int32_t decimalPrecision);
		static uint8_t Float2(const char* label1, const char* label2, const glm::vec2& size, float* values, int32_t decimalPrecision);
		static uint8_t Float3(const char* label1, const char* label2, const char* label3, const glm::vec2& size, float* values, int32_t decimalPrecision);
		static uint8_t Float4(const char* label1, const char* label2, const char* label3, const char* label4, const glm::vec2& size, float* values, int32_t decimalPrecision);

		static uint8_t Int(const char* label, const glm::vec2& size, int32_t& value);
		static uint8_t UInt(const char* label, const glm::vec2& size, uint32_t& value);
		static uint8_t String(const char* label, const glm::vec2& size, std::string& value);


		static uint8_t Image(const char* label, const glm::vec2& size, const Ref<SubTexture>& subTexture);

		static InGuiContext& GetContext();
	};
}