#pragma once
#include "SpriteAnimation.h"

#include <unordered_map>

namespace XYZ {
	/**
	* @class SpriteAnimationController
	* @brief Stores and updates animations
	*/
	class SpriteAnimationController
	{
	public:
		SpriteAnimationController() {};

	private:
		std::unordered_map<std::string, SpriteAnimation> m_Animations;
	};
}