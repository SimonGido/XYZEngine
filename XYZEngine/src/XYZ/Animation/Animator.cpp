#include "stdafx.h"
#include "Animator.h"


namespace XYZ {
	void Animator::Update(Timestep ts)
	{
		for (auto [entity, prop] : m_Properties)
		{
			prop->SetSceneEntity(entity);
		}
		m_Animation->Update(ts);
	}

	void Animator::SetAnimation(const Ref<Animation>& animation)
	{
		m_Animation = animation;
		if (m_Entity.IsValid())
			m_Animation->setSceneEntity(m_Entity);
	}
	void Animator::SetSceneEntity(const SceneEntity& entity)
	{
		m_Entity = entity;
		buildEntityPropertyPairs();
		if (m_Animation.Raw())
			m_Animation->setSceneEntity(m_Entity);
	}
	void Animator::UpdateAnimationEntities()
	{
		buildEntityPropertyPairs();
	}
	void Animator::buildEntityPropertyPairs()
	{
		m_Properties.clear();
		const ECSManager& ecs    = *m_Entity.GetECS();
		const Relationship& rel  = m_Entity.GetComponent<Relationship>();
		std::vector<Entity> tree = rel.GetTree(ecs);

		for (const Entity node : tree)
		{
			SceneEntity nodeEntity(node, m_Entity.GetScene());
			const auto& name = nodeEntity.GetComponent<SceneTagComponent>().Name;
			for (auto& prop : m_Animation->m_Vec4Properties)
			{			
				if (prop.GetPath() == name)
					m_Properties.push_back({ nodeEntity, &prop });
			}
			for (auto& prop : m_Animation->m_Vec3Properties)
			{
				if (prop.GetPath() == name)
					m_Properties.push_back({ nodeEntity, &prop });
			}
			for (auto& prop : m_Animation->m_Vec2Properties)
			{
				if (prop.GetPath() == name)
					m_Properties.push_back({ nodeEntity, &prop });
			}
			for (auto& prop : m_Animation->m_FloatProperties)
			{
				if (prop.GetPath() == name)
					m_Properties.push_back({ nodeEntity, &prop });
			}
			for (auto& prop : m_Animation->m_PointerProperties)
			{
				if (prop.GetPath() == name)
					m_Properties.push_back({ nodeEntity, &prop });
			}
		}
	}
}