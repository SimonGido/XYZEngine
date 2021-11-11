#pragma once
#include "XYZ/Event/Event.h"
#include "XYZ/Event/InputEvent.h"
#include "XYZ/Event/ApplicationEvent.h"
#include "XYZ/Event/EventSystem.h"
#include "XYZ/Renderer/APIContext.h"
#include "WindowCodes.h"

#include <functional>

namespace XYZ {
	enum WindowFlags
	{
		NONE = 1 << 0,
		MAXIMIZED = 1 << 1,
		FULLSCREEN = 1 << 2
	};

	struct WindowProperties
	{
		std::string Title;
		uint32_t Width;
		uint32_t Height;
		int Flags;

		WindowProperties(const std::string& title = "Engine",
			uint32_t width = 1024,
			uint32_t height = 840,
			int flags = WindowFlags::MAXIMIZED)
			: Title(title), Width(width), Height(height), Flags(flags)
		{
		}
	};

	class Window : public EventCaller
	{
	public:
		virtual ~Window() = default;
		virtual void Update() = 0;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;

		// Window attributes
		virtual void SetVSync(int32_t frames) = 0;
		virtual void SetStandardCursor(uint8_t cursor) = 0;
		virtual void SetCustomCursor(void* cursor) = 0;

		virtual void* CreateCustomCursor(uint8_t* pixels, uint32_t width, uint32_t height, int32_t xOffset = 0, int32_t yOffset = 0) = 0;
		virtual bool IsClosed() = 0;
		virtual void* GetWindow() const = 0;
		virtual void* GetNativeWindow() const = 0;

		virtual Ref<APIContext> GetContext() = 0;
		static std::unique_ptr<Window> Create(const WindowProperties& props = WindowProperties());

	};

}