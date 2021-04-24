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
		std::vector<uint32_t> ItemsPerRow;
		bool EraseOut;
	};

	class bUI
	{
	public:
		static void Init();
		static void Shutdown();
		static void Update();

		static void OnEvent(Event& event);
		static void SetupLayout(const std::string& uiName, const std::string& name, const bUILayout& layout);
		static void SetupLayout(bUIAllocator& allocator, bUIElement& element, const bUILayout& layout);
		
		template <typename T>
		static void ForEach(const std::string& uiName, const std::function<void(T&)>& func)
		{
			bUIAllocator& allocator = getContext().Data.GetAllocator(uiName);
			for (size_t i = 0; i < allocator.Size(); ++i)
				if (T* casted = dynamic_cast<T*>(allocator.GetElement<bUIElement>(i)))
					func(*casted);
		}

		template <typename T>
		static void ForEach(const std::string& uiName, const std::string& name, const std::function<void(T&)>& func)
		{
			bUIAllocator& allocator = getContext().Data.GetAllocator(uiName);
			bUIElement* element = allocator.GetElement<bUIElement>(name);
			forEach<T>(allocator, element, func);
		}

		template <typename T>
		static void ForEach(bUIAllocator& allocator, const std::function<void(T&)>& func)
		{
			for (size_t i = 0; i < allocator.Size(); ++i)
				if (T* casted = dynamic_cast<T*>(allocator.GetElement<bUIElement>(i)))
					func(*casted);
		}

		template <typename T>
		static void ForEach(bUIAllocator& allocator, bUIElement* element, const std::function<void(T&)>& func)
		{
			forEach<T>(allocator, element, func);
		}

		template <typename T>
		static T& GetUI(const std::string& uiName, const std::string& name)
		{
			static_assert(std::is_base_of<bUIElement, T>::value, "Type T must inherit from bUIElement");
			return *getContext().Data.GetAllocator(uiName).GetElement<T>(name);
		}
		
		static bUIConfig& GetConfig();
		static const bUIContext& GetContext();
		static bUIAllocator& GetAllocator(const std::string& name);

	private:
		template <typename T>
		static void forEach(bUIAllocator& allocator, bUIElement* element, const std::function<void(T&)>& func)
		{
			Tree& tree = allocator.m_Tree;
			tree.TraverseNodeChildren(element->ID, [&](void* parent, void* child) -> bool {
				bUIElement* childElement = static_cast<bUIElement*>(child);
				if (T* casted = dynamic_cast<T*>(childElement))
					func(*casted);
				forEach<T>(allocator, childElement, func);
				return false;
			});
		}

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