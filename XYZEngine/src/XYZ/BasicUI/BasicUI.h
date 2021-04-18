#pragma once
#include "BasicUIConfig.h"
#include "BasicUIData.h"
#include "BasicUIRenderer.h"

#include "XYZ/Event/InputEvent.h"
#include "XYZ/Event/ApplicationEvent.h"

namespace XYZ {
	
	struct bUIEditData
	{
		bUIElement* Element;
		glm::vec2   MouseOffset;
	};

	struct bUIContext
	{
		bUIConfig   Config;
		bUIData	    Data;
		bUIRenderer Renderer;
		bUIEditData EditData;

		glm::vec2   ViewportSize;
		std::unordered_map<std::string, bUIElement*> ElementMap;
	};

	class bUI
	{
	public:
		static void Init();
		static void Shutdown();
		static void Update();

		static void OnEvent(Event& event);

		static bUIConfig& GetConfig();
		static const bUIContext& GetContext();
	private:
		static bool onWindowResize(WindowResizeEvent& event);
		static bool onMouseButtonPress(MouseButtonPressEvent& event);
		static bool onMouseButtonRelease(MouseButtonReleaseEvent& event);
		static bool onMouseMove(MouseMovedEvent& event);

		static bUIContext& getContext();


		friend class bUILoader;
		friend class bUIRenderer;
	};

}