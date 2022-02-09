#include "stdafx.h"
#include "AnimationAvatar.h"

#include "Animation.h"

namespace XYZ {

	void AnimationAvatar::SetAvatar()
	{
		XYZ_ASSERT(Valid(), "AnimationAvatar is not valid");
		for (auto &[entity, prop] : m_Properties)
		{
			prop->SetSceneEntity(entity);
		}
	}
	bool AnimationAvatar::Create(const SceneEntity& entity, const Ref<Animation>& anim)
	{
		XYZ_ASSERT(entity.IsValid() && anim.Raw(), "Entity must be valid");
		m_Entity = entity;
		m_Animation = anim;

		m_Properties.clear();
		const ECSManager& ecs    = *m_Entity.GetECS();
		const Relationship& rel  =  m_Entity.GetComponent<Relationship>();
		std::vector<Entity> tree =  rel.GetTree(ecs);
		tree.push_back(entity);

		for (const Entity node : tree)
		{
			SceneEntity nodeEntity(node, m_Entity.GetScene());
			if (!Compatible(nodeEntity))
				return false;

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
		return true;
	}
	bool AnimationAvatar::Valid() const
	{
		if (!m_Animation.Raw() || !m_Entity.IsValid())
			return false;

		const ECSManager& ecs = *m_Entity.GetECS();
		const Relationship& rel = m_Entity.GetComponent<Relationship>();
		const std::vector<Entity> tree = rel.GetTree(ecs);

		for (const auto& [entity, property] : m_Properties)
		{
			if (!property->IsCompatible(entity))
				return false;
		}
	}
	bool AnimationAvatar::Compatible(const SceneEntity& entity) const
	{
		bool compatible =
			   propertyCompatible(m_Animation->GetProperties<glm::vec4>(), entity)
			|| propertyCompatible(m_Animation->GetProperties<glm::vec3>(), entity)
			|| propertyCompatible(m_Animation->GetProperties<glm::vec2>(), entity)
			|| propertyCompatible(m_Animation->GetProperties<float>(), entity)
			|| propertyCompatible(m_Animation->GetProperties<void*>(), entity);


		if (!compatible)
		{
			return m_Animation->Empty();
		}
		return true;
	}
}