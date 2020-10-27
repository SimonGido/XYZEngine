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

	static bool Collide(const glm::vec3& translation, const glm::vec3& scale, const glm::vec2& mousePos)
	{
		return (
			mousePos.x > translation.x - scale.x / 2 && mousePos.x < translation.x + scale.x / 2 &&
			mousePos.y > translation.y - scale.y / 2 && mousePos.y < translation.y + scale.y / 2
			);
	}

	SceneHierarchyPanel::SceneHierarchyPanel()
		:
		m_Entities(50, 100)
	{
		InGui::Begin(PanelID::SceneHierarchy, "Scene Hierarchy", { 0,0 }, { 400,300 });
		InGui::End();
		m_Window = InGui::GetWindow(PanelID::SceneHierarchy);
	}

	SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& context)
		:
		m_Context(context),
		m_Entities(50, 100)
	{
		InGui::Begin(PanelID::SceneHierarchy, "Scene Hierarchy", { 0,0 }, { 400,300 });
		InGui::End();
		m_Window = InGui::GetWindow(PanelID::SceneHierarchy);
		m_Window->Flags &= ~InGuiWindowFlag::EventBlocking;
	}
	void SceneHierarchyPanel::SetContext(const Ref<Scene>& context)
	{
		m_Context = context;
		for (auto ent : m_Context->m_Entities)
		{
			Entity entity = { ent,m_Context.Raw() };
			if (entity.HasComponent<SceneTagComponent>())
			{
				auto transform = entity.GetComponent<TransformComponent>();
				
				m_Entities.Insert(entity, transform->Translation, transform->Scale);
			}
		}
	}

	bool SceneHierarchyPanel::OnInGuiRender()
	{
		bool active = false;
		if (InGui::Begin(PanelID::SceneHierarchy, "Scene Hierarchy", { 0,0 }, { 400,300 }))
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
				m_Window->Flags &= ~InGuiWindowFlag::AutoPosition;
				InGui::BeginPopup("New...", m_PopupPosition, { 150.0f,25.0f }, m_PopupEnabled);

				if (InGui::PopupItem("Empty Entity") & InGuiReturnType::Clicked)
				{
					auto entity = m_Context->CreateEntity("New Entity");
					auto transform = entity.GetComponent<TransformComponent>();
					m_Entities.Insert(entity, transform->Translation, transform->Scale);
					m_PopupEnabled = false;
				}

				m_Window->Flags |= InGuiWindowFlag::AutoPosition;
			}

			if (m_Window->Flags & InGuiWindowFlag::Hoovered)
			{
				if (InGui::ResolveRightClick())
				{
					auto [width, height] = Input::GetWindowSize();
					auto [mx, my] = Input::GetMousePosition();

					m_PopupEnabled = !m_PopupEnabled;
					m_PopupPosition = MouseToWorld({ mx,my }, { width,height });
				}
				else if (InGui::ResolveLeftClick())
				{
					m_PopupEnabled = false;
					m_SelectedEntity = Entity();
				}
			}
			active = true;
		}
		InGui::End();

		return active;
	}
	void SceneHierarchyPanel::SelectEntity(const glm::vec2& position)
	{
		Entity* buffer = nullptr;
		size_t count = m_Entities.GetElements(&buffer, position, { 50,50 });
		m_SelectedEntity = Entity();
		if (buffer)
		{
			for (size_t i = 0; i < count; ++i)
			{
				auto transform = buffer[i].GetComponent<TransformComponent>();
				if (Collide(transform->Translation, transform->Scale, position))
				{
					auto &flags = InGui::GetWindow(PanelID::SceneHierarchy)->Flags;
					flags |= InGuiWindowFlag::Modified;
					m_SelectedEntity = buffer[i];
					break;
				}
			}
			delete[]buffer;
		}
	}
	void SceneHierarchyPanel::InvalidateEntity()
	{
		m_SelectedEntity = Entity();
	}

	void SceneHierarchyPanel::RemoveEntity(Entity entity)
	{
		auto transform = entity.GetComponent<TransformComponent>();	
		if (!m_Entities.Remove(entity, transform->Translation, transform->Scale))
				XYZ_LOG_ERR("Entity was not removed from hash grid");		
	}
	void SceneHierarchyPanel::InsertEntity(Entity entity)
	{
		auto transform = entity.GetComponent<TransformComponent>();
		m_Entities.Insert(entity, transform->Translation, transform->Scale);
	}

	void SceneHierarchyPanel::drawEntity(Entity entity)
	{
		if (entity.HasComponent<SceneTagComponent>())
		{
			auto& tag = entity.GetComponent<SceneTagComponent>()->Name;
			glm::vec4 textColor = { 1,1,1,1 };
			if ((uint32_t)m_SelectedEntity == (uint32_t)entity)
			{
				textColor = { 0,1,1,1 };
			}
			
		
			if (InGui::Text(tag.c_str(), textColor) & InGuiReturnType::Clicked)
			{
				m_SelectedEntity = entity;
				auto &flags = InGui::GetWindow(PanelID::SceneHierarchy)->Flags;
				flags |= InGuiWindowFlag::Modified;
			}
			InGui::Separator();
		}
	}
}