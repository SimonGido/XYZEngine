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

		static bool Begin(uint32_t id, const char* name, const glm::vec2& position, const glm::vec2& size);
		static void End();

		static bool BeginPopup(const char* name, glm::vec2& position, const glm::vec2& size, bool& open);
		static bool PopupItem(const char* name, const glm::vec2& size);
		static void EndPopup();

		static bool MenuBar(const char* name,float width, bool& open);
		static bool MenuItem(const char* name, const glm::vec2& size);

		static bool BeginGroup(const char* name,const glm::vec2& position, bool& open);
		static void EndGroup();

		static bool Button(const char* name, const glm::vec2& size);
		static bool Checkbox(const char* name,const glm::vec2 position, const glm::vec2& size, bool& value);
		static bool Slider(const char* name, const glm::vec2 position, const glm::vec2& size, float& value, float valueScale = 1.0f);
		static bool Image(const char* name, uint32_t rendererID, const glm::vec2& position, const glm::vec2& size, const glm::vec4& texCoords = { 0,0,1,1 }, float tilingFactor = 1.0f);
		static bool TextArea(const char* name, std::string& text, const glm::vec2& position, const glm::vec2& size, bool& modified);
		

		static bool Float(uint32_t count, const char* name, float* values, int * lengths, const glm::vec2& position, const glm::vec2& size, int& selected);

		static bool Text(const char* text, const glm::vec2& scale, const glm::vec4& color = { 1,1,1,1 });
		static bool ColorPicker4(const char* name, const glm::vec2& size, glm::vec4& pallete, glm::vec4& color);
		static bool ColorPallete4(const char* name, const glm::vec2& size, glm::vec4& color);
		static bool RenderWindow(uint32_t id, const char* name, uint32_t rendererID, const glm::vec2& position, const glm::vec2& size, float panelSize);

		static bool NodeWindow(uint32_t id, const char* name, const glm::vec2& position, const glm::vec2& size, float dt);
		static void PushConnection(uint32_t start, uint32_t end);
		static bool BeginNode(uint32_t id, std::string& name, const glm::vec2& position, const glm::vec2& size, bool &modified);
		static bool BeginConnection(std::pair<uint32_t, uint32_t>& connection);

		static void EndNode();

		static bool BeginInput(uint32_t id, const glm::vec2& size, const char* name);
		static void EndInput();

		static bool BeginOutput(uint32_t id, const glm::vec2& size, const char* name);
		static void EndOutput();

		static void NodeWindowEnd();
		
		static void Separator();

		static glm::vec4 Selector(bool & selecting);
		static void Selection(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);

		static bool OnWindowResize(const glm::vec2& winSize);
		static bool OnLeftMouseButtonPress();                                                                                                    
		static bool OnRightMouseButtonPress();
		static bool OnLeftMouseButtonRelease();
		static bool OnRightMouseButtonRelease();
		static bool OnMouseScroll();
		
		static bool OnKeyPress(int key, int mod);
		static bool OnKeyRelease();
		static bool IsKeyPressed(int key);

		static InGuiWindow* GetCurrentWindow();
		static InGuiNodeWindow* GetCurrentNodeWindow();
		static InGuiNodeWindow* GetNodeWindow(uint32_t id);	
		static InGuiWindow* GetWindow(uint32_t id);

		static glm::vec2& MouseRelativePosition(const InGuiWindow& window, const glm::vec3& cameraPos);
	private:
		static InGuiWindow* createWindow(uint32_t id, const glm::vec2& position, const glm::vec2& size);
		static InGuiNodeWindow* createNodeWindow(uint32_t id, const glm::vec2& position, const glm::vec2& size);
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