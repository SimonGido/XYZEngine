#include "stdafx.h"
#include "GuiSystem.h"


namespace XYZ {
	GuiSystem::GuiSystem()
	{
		m_Signature.set(ECSManager::Get().GetComponentType<Transform2D>());
		m_Signature.set(ECSManager::Get().GetComponentType<UIComponent>());
	}
	GuiSystem::~GuiSystem()
	{
	}
	void GuiSystem::Update(float dt)
	{
		m_Widgets.Propagate(Propagate());
	}
	void GuiSystem::Add(Entity entity)
	{
		Component component;
		component.ActiveComponent = ECSManager::Get().GetComponent<ActiveComponent>(entity);
		component.Transform = ECSManager::Get().GetComponent<Transform2D>(entity);
		component.UI = ECSManager::Get().GetComponent<UIComponent>(entity);

		component.UI->Index = m_Widgets.GetSize();

		m_Widgets.InsertNode(Node<Component>(component));
		m_Widgets.SetParent(component.UI->Parent, component.UI->Index,Setup());		
	}
	void GuiSystem::Remove(Entity entity)
	{
		std::vector<uint16_t> deleted;
		for (size_t i = 0; i < m_Widgets.GetFlatData().size(); ++i)
		{
			auto& elem = m_Widgets.GetFlatData()[i];
			if (elem.GetData().Ent == entity)
			{
				XYZ_LOG_INFO("Entity with id ", entity, " removed");
				m_Widgets.DeleteNode(elem.GetData().UI->Index);
				break;
			}
		}
	}
	bool GuiSystem::Contains(Entity entity)
	{
		for (size_t i = 0; i < m_Widgets.GetFlatData().size(); ++i)
		{
			auto& elem = m_Widgets.GetFlatData()[i];
			if (elem.GetData().Ent == entity)
			{
				return true;
			}
		}
		return false;
	}
}
