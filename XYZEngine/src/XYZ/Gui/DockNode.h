#pragma once
#include "XYZ/ECS/ECSManager.h"
#include "XYZ/ECS/Entity.h"
#include "XYZ/ECS/Types.h"

#include "XYZ/Event/EventSystem.h"
#include "XYZ/Event/GuiEvent.h"

#include <glm/glm.hpp>


namespace XYZ {

	enum class SplitType
	{
		None,
		Horizontal,
		Vertical
	};

	struct DockNodeComponent : public Type<DockNodeComponent>,
							   public EventSystem<ComponentResizedEvent>
	{
		DockNodeComponent(const glm::vec3& position, const glm::vec2& size);

		std::vector<uint32_t> Entities;
		glm::vec3 Position;
		glm::vec2 Size;
		SplitType Split = SplitType::None;
	
	private:
		bool onResize(ComponentResizedEvent& event);
	};

	struct Dockable : public Type<Dockable>
	{};
}