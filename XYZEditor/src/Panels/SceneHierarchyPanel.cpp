#include "SceneHierarchyPanel.h"

#include "Panel.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>


namespace XYZ {
	static glm::vec2 MouseToWorld(const glm::vec2& point, const glm::vec2& windowSize)
	{
		glm::vec2 offset = { windowSize.x / 2,windowSize.y / 2 };
		return { point.x - offset.x, offset.y - point.y };
	}

	SceneHierarchyPanel::SceneHierarchyPanel(uint32_t id)
		:
		Panel(id)
	{
		InGui::Begin(id, "Scene Hierarchy", { 0,0 }, { 400,300 });
		InGui::End();

		InGui::GetWindow(id)->Flags &= ~InGuiWindowFlag::EventBlocking;
	}

	
	void SceneHierarchyPanel::SetContext(const Ref<Scene>& context)
	{
		m_Context = context;
	}


	void SceneHierarchyPanel::OnInGuiRender()
	{
		bool active = false;
		if (InGui::Begin(m_PanelID, "Scene Hierarchy", { 0,0 }, { 400,300 }))
		{
			if (m_Context)
			{
				for (auto ent : m_Context->m_Entities)
				{
					Entity entity = { ent,m_Context.Raw() };
					drawEntity(entity);
				}
			}		
			if (m_PopupEnabled)
			{
				InGui::GetWindow(m_PanelID)->Flags &= ~InGuiWindowFlag::AutoPosition;
				InGui::BeginPopup("New...", m_PopupPosition, { 150.0f,25.0f }, m_PopupEnabled);
				if (InGui::PopupItem("Empty Entity") & InGuiReturnType::Clicked)
				{
					auto entity = m_Context->CreateEntity("New Entity");
					m_PopupEnabled = false;
				}
				InGui::GetWindow(m_PanelID)->Flags |= InGuiWindowFlag::AutoPosition;
			}	
		}
		InGui::End();
	}
	void SceneHierarchyPanel::OnUpdate(Timestep ts)
	{
	}
	void SceneHierarchyPanel::OnEvent(Event& event)
	{
		// Events that should be called only when scene window is hoovered
		if (InGui::GetWindow(m_PanelID)->Flags & InGuiWindowFlag::Hoovered)
		{
			EventDispatcher dispatcher(event);
			dispatcher.Dispatch<MouseButtonPressEvent>(Hook(&SceneHierarchyPanel::onMouseButtonPress, this));
			dispatcher.Dispatch<MouseButtonReleaseEvent>(Hook(&SceneHierarchyPanel::onMouseButtonRelease, this));
			dispatcher.Dispatch<KeyPressedEvent>(Hook(&SceneHierarchyPanel::onKeyPress, this));
			dispatcher.Dispatch<KeyReleasedEvent>(Hook(&SceneHierarchyPanel::onKeyRelease, this));
		}
	}
	bool SceneHierarchyPanel::onMouseButtonPress(MouseButtonPressEvent& event)
	{
		if (event.IsButtonPressed(MouseCode::XYZ_MOUSE_BUTTON_RIGHT))
		{
			m_PopupEnabled = !m_PopupEnabled;
			auto [width, height] = Input::GetWindowSize();
			auto [mx, my] = Input::GetMousePosition();
			m_PopupPosition = MouseToWorld({ mx,my }, { width,height });
			return true;
		}
		return false;
	}

	bool SceneHierarchyPanel::onMouseButtonRelease(MouseButtonReleaseEvent& event)
	{
		return false;
	}

	bool SceneHierarchyPanel::onKeyPress(KeyPressedEvent& event)
	{
		if (event.IsKeyPressed(KeyCode::XYZ_KEY_DELETE))
		{
			Entity entity = m_Context->GetSelectedEntity();
			if (entity)
			{
				m_Context->DestroyEntity(entity);
				m_Context->SetSelectedEntity(Entity());
				Execute(DeselectedEvent());
				return true;
			}
		}
		return false;
	}

	bool SceneHierarchyPanel::onKeyRelease(KeyReleasedEvent& event)
	{
		return false;
	}
	void SceneHierarchyPanel::drawEntity(Entity entity)
	{
		if (entity.HasComponent<SceneTagComponent>())
		{
			auto& tag = entity.GetComponent<SceneTagComponent>()->Name;
			glm::vec4 textColor = { 1,1,1,1 };
			if ((uint32_t)m_Context->GetSelectedEntity() == (uint32_t)entity)
			{
				textColor = { 0,1,1,1 };
			}	
			if (InGui::Text(tag.c_str(), textColor) & InGuiReturnType::Clicked)
			{
				m_Context->SetSelectedEntity(entity);
				EntitySelectedEvent e(entity);
				Execute(e);
				InGui::GetWindow(m_PanelID)->Flags |= InGuiWindowFlag::Modified;
			}
			InGui::Separator();
		}
	}
}