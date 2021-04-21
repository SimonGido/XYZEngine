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
		bUIConfig    Config;
		bUIData		 Data;
		bUIRenderer  Renderer;
		bUIEditData  EditData;

		glm::vec2    ViewportSize;


	};

	struct bUILayout
	{
		float SpacingX, SpacingY;
		float LeftOffset, RightOffset, TopOffset;
	};

	class bUI
	{
	public:
		static void Init();
		static void Shutdown();
		static void Update();

		static void OnEvent(Event& event);
		static void SetupLayout(const std::string& uiName, const std::string& name, const bUILayout& layout);

		template <typename T>
		static T& GetUI(const std::string& uiName, const std::string& name)
		{
			static_assert(std::is_base_of<bUIElement, T>::value, "Type T must inherit from bUIElement");
			return *getContext().Data.GetAllocator(uiName).GetElement<T>(name);
		}
		

		static bUIConfig& GetConfig();
		static const bUIContext& GetContext();
	private:
		static bool onWindowResize(WindowResizeEvent& event);
		static bool onMouseButtonPress(MouseButtonPressEvent& event);
		static bool onMouseButtonRelease(MouseButtonReleaseEvent& event);
		static bool onMouseMove(MouseMovedEvent& event);
		static bool onMouseScroll(MouseScrollEvent& event);
		static bool onKeyType(KeyTypedEvent& event);
		static bool onKeyPress(KeyPressedEvent& event);

		static bUIContext& getContext();


		friend class bUILoader;
		friend class bUIRenderer;
	};

}