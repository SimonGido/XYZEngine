#pragma once
#include "InGuiWindow.h"
#include "InGuiFrame.h"
#include "InGuiConfig.h"
#include "InGuiInput.h"

#include "XYZ/Renderer/Buffer.h"
#include "XYZ/Renderer/CustomRenderer2D.h"
#include "XYZ/Utils/DataStructures/MemoryPool.h"
#include "XYZ/Event/InputEvent.h"

namespace XYZ {
	class InGuiContext
	{
		using InGuiWindowMap = std::unordered_map<std::string_view, InGuiWindow*>;
	public:
		InGuiContext();

		void Render();
		void SetViewportSize(uint32_t width, uint32_t height);

		void FocusWindow(InGuiWindow* window);

		void OnEvent(Event& event);

		InGuiWindow* CreateWindow(const char* name);
		InGuiWindow* GetWindow(const char* name);

		InGuiInput				  m_Input;
		InGuiFrame				  m_FrameData;
		InGuiConfig				  m_Config;
		uint32_t				  m_ViewportWidth;
		uint32_t				  m_ViewportHeight;

		std::vector<InGuiWindow*> m_Windows;
		InGuiWindowMap			  m_WindowMap;
		InGuiWindow*			  m_FocusedWindow;
		MemoryPool				  m_Pool;
		CustomRenderer2DLayout	  m_RendererLayout;

		InGuiID					  m_LastLeftPressedID;

		std::vector<InGuiRect>    m_ClipRectangles;
		Ref<ShaderStorageBuffer>  m_ClipBuffer;

		static constexpr uint32_t sc_MaxNumberOfClipRectangles = 256;
		friend class InGui;
	};
}