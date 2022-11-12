#pragma once
#include "Core.h"
#include "KeyCodes.h"
#include "MouseCodes.h"

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
	class XYZ_API Input
	{

	public:
		static bool IsKeyPressed(KeyCode button);
		static bool IsMouseButtonPressed(MouseCode button);
	
		static std::pair<float, float> GetMousePosition();

		static float GetMouseX();
		static float GetMouseY();
		static std::pair<int, int> GetWindowSize();
		static std::pair<int, int> GetWindowPosition();
	};
}
 