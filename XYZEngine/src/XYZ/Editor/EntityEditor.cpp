#include "stdafx.h"
#include "EntityEditor.h"


namespace XYZ {
	EntityEditor::EntityEditor(Entity entity)
		:
		m_Entity(entity)
	{
	}
	void EntityEditor::drawComponents()
	{
		if (m_Entity.HasComponent<TransformComponent>())
		{
			auto transform = m_Entity.GetComponent<TransformComponent>();

		}
	}
}