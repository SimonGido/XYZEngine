#pragma once
#include "InGuiStructures.h"
#include "InGuiDockSpace.h"

namespace XYZ {

	struct InGuiContext
	{
		InGuiRenderConfiguration RenderConfiguration;
		InGuiWindowMap Windows;
		InGuiDockSpace* DockSpace = nullptr;
		

		void ActivateSubFrame();
		void ActivateMainFrame() { CurrentFrameData = &MainFrameData; }

		inline InGuiPerFrameData& GetPerFrameData() { return CurrentFrameData->PerFrameData; }
		inline InGuiRenderQueue& GetRenderQueue() { return CurrentFrameData->RenderQueue; }
	
	private:
		InGuiFrameData *CurrentFrameData = &MainFrameData;
		InGuiFrameData MainFrameData;
		InGuiFrameData SubFrameData;
	};

	class InGui
	{
	public:
		static void Init(const InGuiRenderConfiguration& renderConfig);
		static void Destroy();

		static void BeginFrame();
		static void EndFrame();

		static void BeginSubFrame();
		static void EndSubFrame();

		static bool Begin(uint32_t id, const char* name, const glm::vec2& position, const glm::vec2& size);
		static void End();

		static bool BeginPopup(const char* name, glm::vec2& position, const glm::vec2& size, bool& open);
		static bool PopupItem(const char* name);
		static bool PopupExpandItem(const char* name, bool & open);
		static void PopupExpandEnd();
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
		static bool Icon(const glm::vec2& position, const glm::vec2& size, const Ref<SubTexture2D>& subTexture, uint32_t textureID);
		static bool Icon(const char* name, const glm::vec2& position, const glm::vec2& size, const Ref<SubTexture2D>& subTexture, uint32_t textureID, bool hightlight = false);

		static bool Text(const char* text, const glm::vec2& scale, const glm::vec4& color = { 1,1,1,1 });
		static bool ColorPicker4(const char* name, const glm::vec2& size, glm::vec4& pallete, glm::vec4& color);
		static bool ColorPallete4(const char* name, const glm::vec2& size, glm::vec4& color);
		static bool RenderWindow(uint32_t id, const char* name, uint32_t rendererID, const glm::vec2& position, const glm::vec2& size);
		static void Separator();

		static bool BeginNode(const char* name, const glm::vec2& position, const glm::vec2& size);
		static void PushArrow(const glm::vec2& p0, const glm::vec2& p1, const glm::vec2& size);

		static glm::vec4 Selector(bool & selecting);
		static void Selection(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);

		static bool OnWindowResize(const glm::vec2& winSize);
		static bool OnLeftMouseButtonPress();                                                                                                    
		static bool OnRightMouseButtonPress();
		static bool OnLeftMouseButtonRelease();
		static bool OnRightMouseButtonRelease();
		
		static bool OnKeyPress(int key, int mod);
		static bool OnKeyRelease();
		static bool IsKeyPressed(int key);

		
		static void SetInGuiMesh(InGuiMesh* mesh, InGuiLineMesh* lineMesh);
		static void SetViewProjection(const glm::mat4& viewProjection);
		static void SetMousePosition(const glm::vec2& mousePos);
		static void SetUIOffset(float offset);

		static bool ResolveLeftClick(bool handle = true);
		static bool ResolveRightClick(bool handle = true);
		static bool ResolveLeftRelease(bool handle = true);
		static bool ResolveRightRelease(bool handle = true);

		static InGuiWindow* GetCurrentWindow();
		
		static InGuiWindow* GetWindow(uint32_t id);

		static InGuiRenderConfiguration& GetRenderConfiguration();
		static const glm::vec2& GetMousePosition();
		static glm::vec2 MouseRelativePosition(const InGuiWindow& window, const glm::vec3& cameraPos);
		static glm::vec2 GetWorldPosition(const InGuiWindow& window, const glm::vec3& cameraPos, float aspectRatio, float zoomLevel);
		static glm::vec2 GetInGuiPosition(const InGuiWindow& window, const glm::vec3& cameraPos, float aspectRatio, float zoomLevel);
	private:
		static InGuiWindow* createWindow(uint32_t id, const glm::vec2& position, const glm::vec2& size);

		static bool detectResize(InGuiWindow& window);
		static bool detectMoved(InGuiWindow& window);
		static bool detectCollapse(InGuiWindow& window);

		static void resolveResize(InGuiWindow& window);
		static void resolveMove(InGuiWindow& window);

	public:
		

		static void loadDockSpace();
		static void saveDockSpace();
	private:
		static InGuiContext* s_Context;
	};
}