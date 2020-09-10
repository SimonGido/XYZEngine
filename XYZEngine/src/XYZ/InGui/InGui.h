#pragma once
#include "InGuiStructures.h"


namespace XYZ {


	class InGui
	{
	public:
		static void Init(const InGuiRenderConfiguration& renderConfig);
		static void Destroy();

		static void BeginFrame();
		static void EndFrame();

		static bool Begin(const std::string& name, const glm::vec2& position, const glm::vec2& size, const InGuiRenderConfiguration& renderConfig = s_Context->RenderConfiguration);
		static void End();


		static bool BeginPopup(const std::string& name, const glm::vec2& size, bool& open, const InGuiRenderConfiguration& renderConfig = s_Context->RenderConfiguration);
		static bool PopupItem(const std::string& name, const glm::vec2& size, const InGuiRenderConfiguration& renderConfig = s_Context->RenderConfiguration);
		static void EndPopup();

		static bool MenuBar(const std::string& name,float width, bool& open, const InGuiRenderConfiguration& renderConfig = s_Context->RenderConfiguration);
		static bool MenuItem(const std::string& name, const glm::vec2& size, const InGuiRenderConfiguration& renderConfig = s_Context->RenderConfiguration);

		static bool BeginGroup(const std::string& name, bool& open, const InGuiRenderConfiguration& renderConfig = s_Context->RenderConfiguration);
		static void EndGroup();

		static bool Button(const std::string& name, const glm::vec2& size, const InGuiRenderConfiguration& renderConfig = s_Context->RenderConfiguration);
		static bool Checkbox(const std::string& name, const glm::vec2& size, bool& value, const InGuiRenderConfiguration& renderConfig = s_Context->RenderConfiguration);
		static bool Slider(const std::string& name, const glm::vec2& size, float& value, float valueScale = 1.0f, const InGuiRenderConfiguration& renderConfig = s_Context->RenderConfiguration);
		static bool Image(const std::string& name, uint32_t rendererID, const glm::vec2& size, const InGuiRenderConfiguration& renderConfig = s_Context->RenderConfiguration);
		static bool TextArea(const std::string& name, std::string& text, const glm::vec2& size, bool& modified, const InGuiRenderConfiguration& renderConfig = s_Context->RenderConfiguration);

		static bool Text(const std::string& text, const glm::vec2& scale, const glm::vec4& color = { 1,1,1,1 }, const InGuiRenderConfiguration& renderConfig = s_Context->RenderConfiguration);
		static bool ColorPicker4(const std::string& name, const glm::vec2& size, glm::vec4& pallete, glm::vec4& color, const InGuiRenderConfiguration& renderConfig = s_Context->RenderConfiguration);
		static bool ColorPallete4(const std::string& name, const glm::vec2& size, glm::vec4& color, const InGuiRenderConfiguration& renderConfig = s_Context->RenderConfiguration);
		static bool RenderWindow(const std::string& name, uint32_t rendererID, const glm::vec2& position, const glm::vec2& size, float panelSize, InGuiRenderConfiguration& renderConfig = s_Context->RenderConfiguration);

		static void Separator();

		static glm::vec4 Selector();


		static bool OnWindowResize(const glm::vec2& winSize);
		static bool OnLeftMouseButtonPress();
		static bool OnRightMouseButtonPress();

		static bool OnLeftMouseButtonRelease();
		static bool OnRightMouseButtonRelease();
		static bool OnKeyPress(int key, int mod);
		static bool OnKeyRelease();
		
		static const InGuiWindow& GetWindow(const std::string& name) { return *s_Context->Windows[name]; }
		static void SetWindowFlags(const std::string& name, uint16_t flags) { s_Context->Windows[name]->Flags = flags; }
		static uint16_t& GetWindowFlags(const std::string& name) { return s_Context->Windows[name]->Flags; }
	private:
		static InGuiWindow* getWindow(const std::string& name);
		static InGuiWindow* createWindow(const std::string& name,const glm::vec2& position, const glm::vec2& size);

		static bool detectResize(InGuiWindow& window);
		static bool detectMoved(InGuiWindow& window);
		static bool detectCollapse(InGuiWindow& window);

		static void resolveResize(InGuiWindow& window);
		static void resolveMove(InGuiWindow& window);


		static void loadDockSpace();
		static void saveDockSpace();

	private:
		static InGuiContext* s_Context;
	};
}