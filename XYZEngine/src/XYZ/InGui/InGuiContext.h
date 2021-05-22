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
		using InGuiWindowMap = std::unordered_map<InGuiID, InGuiWindow*>;
	public:
		InGuiContext();
		~InGuiContext();

		void Render();
		void SetViewportSize(uint32_t width, uint32_t height);

		void FocusWindow(InGuiWindow* window);

		void OnEvent(Event& event);

		InGuiWindow* CreateInGuiWindow(const char* name);
		InGuiWindow* GetInGuiWindow(const char* name);

		InGuiInput				  m_Input;
		InGuiFrame				  m_FrameData;
		InGuiConfig				  m_Config;
		uint32_t				  m_ViewportWidth;
		uint32_t				  m_ViewportHeight;

		std::vector<InGuiWindow*> m_Windows;
		InGuiWindowMap			  m_WindowMap;
		InGuiWindow*			  m_FocusedWindow;
		MemoryPool				  m_WindowPool;
		CustomRenderer2DLayout	  m_RendererLayout;

		InGuiID					  m_LastLeftPressedID;
		InGuiID					  m_LastHooveredID;
		InGuiID					  m_LastInputID;

		InGuiID					  m_MenuOpenID;
		bool					  m_MenuBarActive;

		std::string				  m_TemporaryTextBuffer;
		std::vector<InGuiRect>    m_ClipRectangles;
		Ref<ShaderStorageBuffer>  m_ClipBuffer;

		static constexpr uint32_t sc_MaxNumberOfClipRectangles = 256;
		static constexpr size_t sc_InputValueBufferSize = 64;
		static constexpr size_t sc_SliderValueBufferSize = 64;
		friend class InGui;
	};
	
}