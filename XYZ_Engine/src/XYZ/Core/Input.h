#pragma once
#include <glm/glm.hpp>
#include <utility>
#include <memory>


/**	@class Input
 *  Singleton class with virtual implementation, OS dependent.
 *	To access the instance and it's functionality are used static functions
 *	Keeps track of user input.
 *
 */
namespace XYZ {
	class Input
	{
	protected:
		Input() = default;
	public:
		/**
		* Delete copy constructor
		*/
		Input(const Input&) = delete;
		/**
		* Delete assign operator
		*/
		Input& operator=(const Input&) = delete;

		inline static bool IsKeyPressed(int keycode) { return s_Instance->IsKeyPressedImpl(keycode); }
		inline static bool IsMouseButtonPressed(int button) { return s_Instance->IsMouseButtonPressedImpl(button); }
		inline static std::pair<float, float> GetMousePosition() { return s_Instance->GetMousePositionImpl(); }

		inline static float GetMouseX() { return s_Instance->GetMouseXImpl(); }
		inline static float GetMouseY() { return s_Instance->GetMouseYImpl(); }
		inline static std::pair<int, int> GetWindowSize() { return s_Instance->GetWindowSizeImpl(); }
	protected:
		virtual bool IsKeyPressedImpl(int keycode) = 0;

		virtual bool IsMouseButtonPressedImpl(int button) = 0;
		virtual std::pair<float, float> GetMousePositionImpl() = 0;
		virtual float GetMouseXImpl() = 0;
		virtual float GetMouseYImpl() = 0;
		virtual std::pair<int, int> GetWindowSizeImpl() = 0;
	private:
		static std::unique_ptr<Input> s_Instance;
	};
}
 