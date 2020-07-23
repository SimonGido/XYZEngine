#include "stdafx.h"
#include "GuiSystem.h"


#include "XYZ/Core/Input.h"
#include "XYZ/Event/GuiEvent.h"

#include "Button.h"
#include "Checkbox.h"

namespace XYZ {
	GuiSystem::GuiSystem(ECSManager* ecs)
		:
		m_ECS(ecs),
		m_Grid(1,50)
	{
		m_Signature.set(m_ECS->GetComponentType<Transform2D>());
		m_Signature.set(m_ECS->GetComponentType<Checkbox>());
	}
	GuiSystem::~GuiSystem()
	{
	}
	void GuiSystem::Update(float dt)
	{
		for (auto& component : m_Components)
			component.UI->OnUpdate(dt);
	}
	void GuiSystem::Add(uint32_t entity)
	{
		Component component;
		component.ActiveComponent = m_ECS->GetComponent<ActiveComponent>(entity);
		component.Transform = m_ECS->GetComponent<Transform2D>(entity);
		component.UI = m_ECS->GetComponent<Checkbox>(entity);
		

		m_Components.push_back(component);
		glm::vec2 pos = { component.Transform->GetWorldPosition().x,component.Transform->GetWorldPosition().y };
		m_Grid.Insert(m_Components.size() - 1, { fabs(pos.x),fabs(pos.y) }, component.Transform->GetWorldScale());
		
		XYZ_LOG_INFO("Entity with ID ", entity, " added");
	}
	void GuiSystem::Remove(uint32_t entity)
	{
		
	}
	bool GuiSystem::Contains(uint32_t entity)
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

		if (m_LastHoovered)
			m_LastHoovered->OnEvent(UnHooverEvent{});

		for (size_t i = 0; i < count; ++i)
		{
			auto transform = m_Components[buffer[i]].Transform;
			if (collide(transform->GetWorldPosition(), transform->GetWorldScale(), mousePos))
			{
				m_Components[buffer[i]].UI->OnEvent(HooverEvent{});
				m_LastHoovered = m_Components[buffer[i]].UI;
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
