#pragma once
#include "XYZ/ECS/ECSManager.h"
#include "XYZ/Event/EventSystem.h"
#include "XYZ/Event/GuiEvent.h"


#include <glm/glm.hpp>

namespace XYZ {

	struct Padding
	{
		float Left = 0.0f, Right = 0.0f, Top = 0.0f, Bottom = 0.0f;
	};
	
	class LayoutGroup : public ECS::Type<LayoutGroup>,
						public EventSystem<CanvasRendererRebuildEvent>
	{
	public:
		LayoutGroup() {};
		
		Padding Padding;

		glm::vec2 CellSize = glm::vec2(0.0f);
		glm::vec2 CellSpacing = glm::vec2(0.0f);

	};
}