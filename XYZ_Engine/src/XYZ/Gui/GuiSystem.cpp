#include "stdafx.h"
#include "GuiSystem.h"


#include "XYZ/Core/Input.h"
#include "XYZ/Event/GuiEvent.h"

#include "Button.h"

namespace XYZ {
	GuiSystem::GuiSystem()
		:
		m_Grid(1,50)
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
		component.UI = ECSManager::Get().GetComponent<Button>(entity);
		

		m_Components.push_back(component);
		glm::vec2 pos = { component.Transform->GetWorldPosition().x,component.Transform->GetWorldPosition().y };
		m_Grid.Insert(m_Components.size() - 1, { fabs(pos.x),fabs(pos.y) }, component.Transform->GetWorldScale());
		
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
	
	bool GuiSystem::OnMouseButtonPress(const glm::vec2& mousePos)
	{
		size_t* buffer = nullptr;
		size_t count = m_Grid.GetElements(&buffer, { fabs(mousePos.x),fabs(mousePos.y) }, { 1,1 });
		
		for (size_t i = 0; i < count; ++i)
		{
			auto transform = m_Components[buffer[i]].Transform;
			if (collide(transform->GetWorldPosition(), transform->GetWorldScale(), mousePos))
			{
				m_Components[buffer[i]].UI->OnEvent(ClickEvent{});
				m_LastPressed = m_Components[buffer[i]].UI;
				delete[]buffer;
				return true;
			}
		}
		
		delete[]buffer;
		return false;
	}
	bool GuiSystem::OnMouseButtonRelease(const glm::vec2& mousePos)
	{
		if (m_LastPressed)
		{
			m_LastPressed->OnEvent(ReleaseEvent{});
			return true;
		}
		m_LastPressed = nullptr;
		return false;
	}
	bool GuiSystem::OnMouseMove(const glm::vec2& mousePos)
	{
		size_t* buffer = nullptr;
		size_t count = m_Grid.GetElements(&buffer, { fabs(mousePos.x),fabs(mousePos.y) }, { 1,1 });


		for (size_t i = 0; i < count; ++i)
		{
			auto transform = m_Components[buffer[i]].Transform;
			if (collide(transform->GetWorldPosition(), transform->GetWorldScale(), mousePos))
			{
				m_Components[buffer[i]].UI->OnEvent(HooverEvent{});
				delete[]buffer;
				return true;
			}
		}

		delete[]buffer;
		return false;
	}
	bool GuiSystem::OnMouseScroll(const glm::vec2& mousePos)
	{
		return false;
	}
	bool GuiSystem::collide(const glm::vec2& pos, const glm::vec2& size, const glm::vec2& point)
	{
		return (pos.x + (size.x / 2) >= point.x &&
				pos.x - (size.x / 2) <= point.x &&
				pos.y + (size.y / 2) >= point.y &&
				pos.y - (size.y / 2) <= point.y);
	}

}
