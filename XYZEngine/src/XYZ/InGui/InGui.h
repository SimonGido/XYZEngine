#pragma once
#include "InGuiStructures.h"
#include "InGuiDockSpace.h"

namespace XYZ {
	struct InGuiContext
	{
		InGuiRenderConfiguration RenderConfiguration;
		InGuiPerFrameData PerFrameData;
		InGuiWindowMap Windows;
		InGuiNodeWindowMap NodeWindows;
		InGuiRenderQueue RenderQueue;
		InGuiDockSpace* DockSpace = nullptr;
	};

	class InGui
	{
	public:
		static void Init(const InGuiRenderConfiguration& renderConfig);
		static void Destroy();

		static void BeginFrame();
		static void EndFrame();

		static bool Begin(const std::string& name, const glm::vec2& position, const glm::vec2& size);
		static void End();

		static bool BeginPopup(const std::string& name, glm::vec2& position, const glm::vec2& size, bool& open);
		static bool PopupItem(const std::string& name, const glm::vec2& size);
		static void EndPopup();

		static bool MenuBar(const std::string& name,float width, bool& open);
		static bool MenuItem(const std::string& name, const glm::vec2& size);

		static bool BeginGroup(const std::string& name, bool& open);
		static void EndGroup();

		static bool Button(const std::string& name, const glm::vec2& size);
		static bool Checkbox(const std::string& name, const glm::vec2& size, bool& value);
		static bool Slider(const std::string& name, const glm::vec2& size, float& value, float valueScale = 1.0f);
		static bool Image(const std::string& name, uint32_t rendererID, const glm::vec2& size);
		static bool TextArea(const std::string& name, std::string& text, const glm::vec2& size, bool& modified);

		static bool Text(const std::string& text, const glm::vec2& scale, const glm::vec4& color = { 1,1,1,1 });
		static bool ColorPicker4(const std::string& name, const glm::vec2& size, glm::vec4& pallete, glm::vec4& color);
		static bool ColorPallete4(const std::string& name, const glm::vec2& size, glm::vec4& color);
		static bool RenderWindow(const std::string& name, uint32_t rendererID, const glm::vec2& position, const glm::vec2& size, float panelSize);

		static bool NodeWindow(const std::string& name, const glm::vec2& position, const glm::vec2& size, float dt);
		static void PushConnection(uint32_t start, uint32_t end);
		static bool BeginNode(uint32_t id,const std::string& name, const glm::vec2& position, const glm::vec2& size);
		static bool BeginConnection(std::pair<uint32_t, uint32_t>& connection);

		static void EndNode();

		static bool BeginInput(uint32_t id, const glm::vec2& size, const std::string& name);
		static void EndInput();

		static bool BeginOutput(uint32_t id, const glm::vec2& size, const std::string& name);
		static void EndOutput();

		static void NodeWindowEnd();
		
		static void Separator();

		static glm::vec4 Selector();


		static bool OnWindowResize(const glm::vec2& winSize);
		static bool OnLeftMouseButtonPress();                                                                                                    
		static bool OnRightMouseButtonPress();

		static bool OnLeftMouseButtonRelease();
		static bool OnRightMouseButtonRelease();
		static bool OnKeyPress(int key, int mod);
		static bool OnKeyRelease();
		static bool IsKeyPressed(int key);

		static InGuiWindow* GetCurrentWindow();
		static InGuiNodeWindow* GetCurrentNodeWindow();
		static InGuiNodeWindow& GetNodeWindow(const std::string& name) { return *s_Context->NodeWindows[name]; }
		static InGuiWindow& GetWindow(const std::string& name) { return *s_Context->Windows[name]; }
		static glm::vec2& MouseRelativePosition(const InGuiWindow& window, const glm::vec3& cameraPos);
	private:
		static InGuiWindow* getWindow(const std::string& name);
		static InGuiWindow* createWindow(const std::string& name,const glm::vec2& position, const glm::vec2& size);
		static InGuiNodeWindow* getNodeWindow(const std::string& name);
		static InGuiNodeWindow* createNodeWindow(const std::string& name, const glm::vec2& position, const glm::vec2& size);
		static InGuiNode* createNode(uint32_t id, const glm::vec2& position, const glm::vec2& size);
		static InGuiNode* getNode(uint32_t id);

		static bool detectResize(InGuiWindow& window);
		static bool detectMoved(InGuiWindow& window);
		static bool detectCollapse(InGuiWindow& window);

		static void resolveResize(InGuiWindow& window);
		static void resolveMove(InGuiWindow& window);

		static bool resolveLeftClick(bool handle = true);
		static bool resolveRightClick(bool handle = true);
		static bool resolveLeftRelease(bool handle = true);
		static bool resolveRightRelease(bool handle = true);

		static void loadDockSpace();
		static void saveDockSpace();

	private:
		static InGuiContext* s_Context;
	};
}