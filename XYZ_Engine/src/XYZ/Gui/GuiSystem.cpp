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
		m_Signature.set(ECSManager::Get().GetComponentType<UI>());
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
	

		auto ui = ECSManager::Get().GetComponent<UI>(entity);
		switch (ui->Type)
		{
		case WidgetType::None:
			XYZ_ASSERT(false, "Widget type None");
		case WidgetType::Button:
			component.UI = ECSManager::Get().GetComponent<Button>(entity);
		}
		
		

		m_Components.push_back(component);
		glm::vec2 pos = { component.Transform->GetPosition().x,component.Transform->GetPosition().y };
		glm::vec2 size = { component.Transform->GetScale().x,component.Transform->GetScale().y };
		m_Grid.Insert(m_Components.size() - 1, { fabs(pos.x),fabs(pos.y) }, size);
		
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
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<MouseButtonPressEvent>(Hook(&GuiSystem::onMouseButtonPress, this));
		dispatcher.Dispatch<MouseButtonReleaseEvent>(Hook(&GuiSystem::onMouseButtonRelease, this));
		dispatcher.Dispatch<MouseMovedEvent>(Hook(&GuiSystem::onMouseMove, this));
	}
	void GuiSystem::onMouseButtonPress(MouseButtonPressEvent& event)
	{
		auto [x, y] = Input::GetMousePosition();
		auto [width,height] = Input::GetWindowSize();

		glm::vec2 pos = { 0,0 };
		x = (x / width) * 4 - 4 * 0.5f;
		y = 4 * 0.5f - (y / height) * 4;
	
		std::cout << "X: " << x << " Y: " << y << std::endl;

		size_t* buffer = nullptr;
		size_t count = m_Grid.GetElements(&buffer, { fabs(x),fabs(y) }, { 1,1 });

		
		for (size_t i = 0; i < count; ++i)
		{
			auto transform = m_Components[buffer[i]].Transform;
			if (collide(transform->GetPosition(), transform->GetScale(), { x,y }))
			{
				m_Components[buffer[i]].UI->OnEvent(ClickEvent{});
				m_Pressed = buffer[i];
			}
		}
		
		delete[]buffer;
	}
	void GuiSystem::onMouseButtonRelease(MouseButtonReleaseEvent& event)
	{
		m_Components[m_Pressed].UI->OnEvent(ReleaseEvent{});
	}
	void GuiSystem::onMouseMove(MouseMovedEvent& event)
	{
		auto [x, y] = Input::GetMousePosition();
		auto [width, height] = Input::GetWindowSize();

		glm::vec2 pos = { 0,0 };
		x = (x / width) * 4 - 4 * 0.5f;
		y = 4 * 0.5f - (y / height) * 4;


		size_t* buffer = nullptr;
		size_t count = m_Grid.GetElements(&buffer, { fabs(x),fabs(y) }, { 1,1 });


		for (size_t i = 0; i < count; ++i)
		{
			auto transform = m_Components[buffer[i]].Transform;
			if (collide(transform->GetPosition(), transform->GetScale(), { x,y }))
			{
				m_Components[buffer[i]].UI->OnEvent(HooverEvent{});
			}
		}
	}
	void GuiSystem::onMouseScroll(MouseScrollEvent& event)
	{
	}
	bool GuiSystem::collide(const glm::vec2& pos, const glm::vec2& size, const glm::vec2& point)
	{
		return (pos.x + (size.x / 2) >= point.x &&
				pos.x - (size.x / 2) <= point.x &&
				pos.y + (size.y / 2) >= point.y &&
				pos.y - (size.y / 2) <= point.y);
	}

}
