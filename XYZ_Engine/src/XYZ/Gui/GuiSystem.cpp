#include "stdafx.h"
#include "GuiSystem.h"

#include "Button.h"

namespace XYZ {
	GuiSystem::GuiSystem()
	{
		m_Signature.set(ECSManager::Get().GetComponentType<Transform2D>());
		m_Signature.set(ECSManager::Get().GetComponentType<Button>());
	}
	GuiSystem::~GuiSystem()
	{
	}
	void GuiSystem::Update(float dt)
	{
		
	}
	void GuiSystem::Add(Entity entity)
	{
		Component component;
		component.ActiveComponent = ECSManager::Get().GetComponent<ActiveComponent>(entity);
		component.Transform = ECSManager::Get().GetComponent<Transform2D>(entity);
		component.Widg = ECSManager::Get().GetComponent<Button>(entity);

		m_Components.push_back(component);
		
		XYZ_LOG_INFO("Entity with ID ", entity, " added");
	}
	void GuiSystem::Remove(Entity entity)
	{
		
	}
	bool GuiSystem::Contains(Entity entity)
	{
		auto it = std::find(m_Components.begin(), m_Components.end(), entity);
		if (it != m_Components.end())
			return true;
		return false;
	}
	void GuiSystem::OnEvent(Event& event)
	{
		for (auto& it : m_Components)
		{
			it.Widg->OnEvent(event);
		}
	}
}
