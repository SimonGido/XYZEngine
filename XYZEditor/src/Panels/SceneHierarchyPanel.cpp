#include "SceneHierarchyPanel.h"


#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>


namespace XYZ {

	static bool Collide(const glm::mat4& transform, const glm::vec2& mousePos)
	{
		glm::vec3 scale, translation, skew;
		glm::vec4 perspective;
		glm::quat orientation;
		glm::decompose(transform, scale, orientation, translation, skew, perspective);

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
				auto& transform = entity.GetComponent<TransformComponent>()->Transform;
				glm::vec3 scale, translation, skew;
				glm::vec4 perspective;
				glm::quat orientation;
				glm::decompose(transform, scale, orientation, translation, skew, perspective);

				m_Entities.Insert(entity, { translation.x,translation.y }, { scale.x,scale.y });
			}
		}
	}

	void SceneHierarchyPanel::OnInGuiRender()
	{
		if (InGui::Begin("Scene Hierarchy", { 0,0 }, { 400,300 }))
		{
			if (m_Context)
			{
				for (auto ent : m_Context->m_SceneGraph.GetFlatData())
				{
					Entity entity = { ent.GetData().Entity,m_Context.Raw() };
					drawEntity(entity);
				}
			}
		}
		InGui::End();
	}
	void SceneHierarchyPanel::SelectEntity(const glm::vec2& position)
	{
		Entity* buffer = nullptr;
		size_t count = m_Entities.GetElements(&buffer, position, { 50,50 });

		if (buffer)
		{
			for (size_t i = 0; i < count; ++i)
			{
				if (Collide(buffer[i].GetComponent<TransformComponent>()->Transform, position))
				{
					auto &flags = InGui::GetWindow("scene hierarchy").Flags;
					flags |= Modified;
					m_SelectedEntity = buffer[i];
				}
			}
			delete[]buffer;
		}
	}
	void SceneHierarchyPanel::RemoveEntity(Entity entity)
	{
		auto& transform = entity.GetComponent<TransformComponent>()->Transform;
		glm::vec3 scale, translation, skew;
		glm::vec4 perspective;
		glm::quat orientation;
		glm::decompose(transform, scale, orientation, translation, skew, perspective);

		m_Entities.Remove(entity, { translation.x,translation.y }, { scale.x,scale.y });
	}
	void SceneHierarchyPanel::InsertEntity(Entity entity)
	{
		auto& transform = entity.GetComponent<TransformComponent>()->Transform;
		glm::vec3 scale, translation, skew;
		glm::vec4 perspective;
		glm::quat orientation;
		glm::decompose(transform, scale, orientation, translation, skew, perspective);

		m_Entities.Insert(entity, { translation.x,translation.y }, { scale.x,scale.y });
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
				auto transformComponent = entity.GetComponent<TransformComponent>();
				glm::vec3 scale, translation, skew;
				glm::vec4 perspective;
				glm::quat orientation;
				glm::decompose(transformComponent->Transform, scale, orientation, translation, skew, perspective);

				glm::vec3 topLeft = { translation.x - scale.x / 2,translation.y + scale.y / 2,1 };
				glm::vec3 topRight = { translation.x + scale.x / 2,translation.y + scale.y / 2,1 };
				glm::vec3 bottomLeft = { translation.x - scale.x / 2,translation.y - scale.y / 2,1 };
				glm::vec3 bottomRight = { translation.x + scale.x / 2,translation.y - scale.y / 2,1 };

				Renderer2D::SubmitLine(topLeft, topRight);
				Renderer2D::SubmitLine(topRight, bottomRight);
				Renderer2D::SubmitLine(bottomRight, bottomLeft);
				Renderer2D::SubmitLine(bottomLeft, topLeft);
			}
			if (InGui::Text(tag, { 1,1 }, textColor))
			{
				m_SelectedEntity = entity;
				auto &flags = InGui::GetWindow("scene hierarchy").Flags;
				flags |= Modified;
			}
			InGui::Separator();
		}
	}
}