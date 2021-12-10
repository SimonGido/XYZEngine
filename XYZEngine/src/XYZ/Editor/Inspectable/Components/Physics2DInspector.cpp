#include "stdafx.h"
#include "Physics2DInspector.h"

#include "XYZ/Editor/EditorHelper.h"
#include "XYZ/Scene/Components.h"

#include "XYZ/Renderer/Renderer2D.h"

namespace XYZ {

	RigidBody2DInspector::RigidBody2DInspector()
		:
		Inspectable("RigidBody2DInspector")
	{
	}

	bool RigidBody2DInspector::OnEditorRender()
	{
		return EditorHelper::DrawComponent<RigidBody2DComponent>("Rigid Body2D", m_Context, [&](auto& component) {

			if (ImGui::Button("Type"))
				ImGui::OpenPopup("Body Type");
			if (ImGui::BeginPopup("Body Type"))
			{
				if (ImGui::MenuItem("Static"))
				{
					component.Type = RigidBody2DComponent::BodyType::Static;
					ImGui::CloseCurrentPopup();
				}
				if (ImGui::MenuItem("Dynamic"))
				{
					component.Type = RigidBody2DComponent::BodyType::Dynamic;
					ImGui::CloseCurrentPopup();
				}
				if (ImGui::MenuItem("Kinematic"))
				{
					component.Type = RigidBody2DComponent::BodyType::Kinematic;
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}

			ImGui::SameLine();
			if (component.Type == RigidBody2DComponent::BodyType::Static)
				ImGui::Text("Static");
			else if (component.Type == RigidBody2DComponent::BodyType::Dynamic)
				ImGui::Text("Dynamic");
			else if (component.Type == RigidBody2DComponent::BodyType::Kinematic)
				ImGui::Text("Kinematic");
		});
	}

	void RigidBody2DInspector::SetSceneEntity(const SceneEntity& entity)
	{
		m_Context = entity;
	}

	BoxCollider2DInspector::BoxCollider2DInspector()
		:
		Inspectable("BoxCollider2DInspector")
	{
	}

	bool BoxCollider2DInspector::OnEditorRender()
	{
		return EditorHelper::DrawComponent<BoxCollider2DComponent>("Box Collider2D", m_Context, [&](auto& component) {
			/*
			auto [translation, rotation, scale] = m_Context.GetComponent<TransformComponent>().GetWorldComponents();
			const glm::vec3 boxTranslation =
				translation + glm::vec3(component.Offset.x, component.Offset.y, 0.0f);
			renderer->SubmitAABB(
				boxTranslation - glm::vec3(component.Size / 2.0f, 0.0f),
				boxTranslation + glm::vec3(component.Size / 2.0f, 0.0f),
				sc_ColliderColor
			);

			EditorHelper::DrawVec2Control("Size", component.Size);
			EditorHelper::DrawVec2Control("Offset", component.Offset);

			EditorHelper::BeginColumns("Density");
			ImGui::PushItemWidth(75.0f);
			ImGui::InputFloat("##Density", &component.Density);
			ImGui::PopItemWidth();
			EditorHelper::EndColumns();


			EditorHelper::BeginColumns("Friction");
			ImGui::PushItemWidth(75.0f);
			ImGui::InputFloat("##Friction", &component.Friction);
			ImGui::PopItemWidth();
			EditorHelper::EndColumns();
			*/
		});
	}

	void BoxCollider2DInspector::SetSceneEntity(const SceneEntity& entity)
	{
		m_Context = entity;
	}

	CircleCollider2DInspector::CircleCollider2DInspector()
		:
		Inspectable("CircleCollider2DInspector")
	{
	}

	bool CircleCollider2DInspector::OnEditorRender()
	{
		return EditorHelper::DrawComponent<CircleCollider2DComponent>("Circle Collider2D", m_Context, [&](auto& component) {

			/*
			auto [translation, rotation, scale] = m_Context.GetComponent<TransformComponent>().GetWorldComponents();
			const glm::vec3 circleTranslation =
				translation + glm::vec3(component.Offset.x, component.Offset.y, 0.0f);

			renderer->SubmitCircle(circleTranslation, component.Radius, 20, sc_ColliderColor);
			EditorHelper::DrawVec2Control("Offset", component.Offset);

			EditorHelper::BeginColumns("Radius");
			ImGui::PushItemWidth(75.0f);
			ImGui::InputFloat("##Radius", &component.Radius);
			ImGui::PopItemWidth();
			EditorHelper::EndColumns();

			EditorHelper::BeginColumns("Density");
			ImGui::PushItemWidth(75.0f);
			ImGui::InputFloat("##Density", &component.Density);
			ImGui::PopItemWidth();
			EditorHelper::EndColumns();


			EditorHelper::BeginColumns("Friction");
			ImGui::PushItemWidth(75.0f);
			ImGui::InputFloat("##Friction", &component.Friction);
			ImGui::PopItemWidth();
			EditorHelper::EndColumns();
			*/
		});
	}

	void CircleCollider2DInspector::SetSceneEntity(const SceneEntity& entity)
	{
		m_Context = entity;
	}

	ChainCollider2DInspector::ChainCollider2DInspector()
		:
		Inspectable("ChainCollider2DInspector")
	{
	}

	bool ChainCollider2DInspector::OnEditorRender()
	{
		return EditorHelper::DrawComponent<ChainCollider2DComponent>("Chain Collider2D", m_Context, [&](auto& component) {
			/*
			const TransformComponent& transform = m_Context.GetComponent<TransformComponent>();
			auto [translation, rotation, scale] = transform.GetWorldComponents();
			glm::vec3 p0(0.0f);
			glm::vec3 p1(0.0f);
			for (size_t i = 1; i < component.Points.size(); ++i)
			{
				p0 = { component.Points[i - 1].x, component.Points[i - 1].y, 0.0f };
				p1 = { component.Points[i].x, component.Points[i].y, 0.0f };
				p0 += translation;
				p1 += translation;
				glm::vec2 normal = glm::normalize(glm::vec2(p1.y - p0.y, -(p1.x - p0.x)));
				glm::vec3 center = p0 + (p1 - p0) / 2.0f;

				renderer->SubmitLine(center, center + glm::vec3(normal.x, normal.y, 0.0f), sc_ColliderColor);
				renderer->SubmitLine(p0, p1, sc_ColliderColor);
			}

			EditorHelper::BeginColumns("Size");
			ImGui::PushItemWidth(75.0f);
			int size = (int)component.Points.size();
			if (ImGui::InputInt("##Size", &size))
			{
				if (size < 2) size = 2;
				if (size >= 0 && (size_t)size != component.Points.size())
				{
					component.Points.resize(size);
				}
			}
			ImGui::PopItemWidth();
			EditorHelper::EndColumns();


			uint32_t counter = 0;
			for (auto& point : component.Points)
			{
				EditorHelper::DrawVec2Control(std::to_string(counter), point);
				counter++;
			}
			*/
		});
	}

	void ChainCollider2DInspector::SetSceneEntity(const SceneEntity& entity)
	{
		m_Context = entity;
	}

}