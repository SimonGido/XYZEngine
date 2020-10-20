#include "SceneHierarchyPanel.h"

#include "Panel.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>


namespace XYZ {

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
		//InGui::Begin("Scene Hierarchy", { 0,0 }, { 400,300 });
		//InGui::End();
	}

	SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& context)
		:
		m_Context(context),
		m_Entities(50, 100)
	{
		//InGui::Begin("Scene Hierarchy", { 0,0 }, { 400,300 });
		//InGui::End();
	}
	void SceneHierarchyPanel::SetContext(const Ref<Scene>& context)
	{
		m_Context = context;
		for (auto ent : m_Context->m_SceneGraph.GetFlatData())
		{
			Entity entity = { ent.GetData().Entity,m_Context.Raw() };
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
				for (auto ent : m_Context->m_SceneGraph.GetFlatData())
				{
					Entity entity = { ent.GetData().Entity,m_Context.Raw() };
					drawEntity(entity);
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
				}
			}
			delete[]buffer;
		}
	}
	void SceneHierarchyPanel::RemoveEntity(Entity entity)
	{
		auto transform = entity.GetComponent<TransformComponent>();	
		m_Entities.Remove(entity, transform->Translation, transform->Scale);
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
			
		
			if (InGui::Text(tag.c_str(), { 1,1 }, textColor) & InGuiReturnType::Clicked)
			{
				m_SelectedEntity = entity;
				auto &flags = InGui::GetWindow(PanelID::SceneHierarchy)->Flags;
				flags |= InGuiWindowFlag::Modified;
			}
			InGui::Separator();
		}
	}
}