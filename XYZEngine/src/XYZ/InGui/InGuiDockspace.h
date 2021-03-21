#pragma once

#include <glm/glm.hpp>

#include "InGuiStructures.h"

namespace XYZ {



	class InGuiDockspace
	{
	public:
		static void Init(InGuiDockNode* root);
		static void Init(const glm::vec2& pos, const glm::vec2& size);
		static void Destroy();

		static void SetRootSize(const glm::vec2& size);


	private:
		static void beginFrame(InGuiContext* context, bool preview);
		static void endFrame(InGuiContext* context);
		static bool insertWindow(uint32_t id, const glm::vec2& mousePos);
		static bool removeWindow(uint32_t id);
		
		static bool onMouseLeftPress(const glm::vec2& mousePos);
		static bool onMouseLeftRelease();

		static InGuiDockNode* getRoot();

		friend class InGui;
	};
}