#include "stdafx.h"
#include "AnimationPlayer.h"

#include "Animation.h"
#include "Property.h"

namespace XYZ {

	 template <typename T>
	 static uint32_t PropertyCompatible(const std::vector<Property<T>>& props, const std::string_view path)
	 {
		 uint32_t count = 0;
		 for (const auto& pr : props)
		 {
			 if (pr.GetPath() == path)
				 count++;
		 }
		 return count;
	 }


	AnimationPlayer::AnimationPlayer()
		:
		m_CurrentTime(0.0f)
	{
	}
	void AnimationPlayer::Update(Timestep ts)
	{
		XYZ_ASSERT(Valid(), "AnimationPlayer is not valid");
		if (!m_IsCreated)
			return;

		uint32_t frame = GetCurrentFrame();
		if (frame >= m_Animation->GetNumFrames() && m_Animation->GetRepeat())
		{
			m_CurrentTime = 0.0f;
			for (auto& node : m_Properties)
				node.Key = 0;
			frame = 0;
		}

		for (auto& node : m_Properties)
		{
			if (node.Key + 1 < node.Property->GetKeyCount())
			{
				node.Property->SetSceneEntity(node.Entity);
				node.Property->Update(node.Key, frame);
				if (node.Property->GetEndFrame(node.Key + 1) <= frame)
					node.Key++;
			}
		}
		m_CurrentTime += ts;		
	}
	bool AnimationPlayer::Create(const SceneEntity& entity, const Ref<Animation>& anim)
	{
		XYZ_ASSERT(entity.IsValid() && anim.Raw(), "Entity must be valid");
	
		m_Animation = anim;
		if (!Compatible(entity))
		{
			m_IsCreated = false;
			return false;
		}
		m_Entity = entity;

		m_Properties.clear();
		const ECSManager& ecs    = *m_Entity.GetECS();
		const Relationship& rel  =  m_Entity.GetComponent<Relationship>();
		std::vector<Entity> tree =  rel.GetTree(ecs);
		tree.insert(tree.begin(), entity);

		for (const Entity node : tree)
		{
			SceneEntity nodeEntity(node, m_Entity.GetScene());
			const auto& name = nodeEntity.GetComponent<SceneTagComponent>().Name;
			for (auto& prop : m_Animation->m_QuatProperties)
			{
				if (prop.GetPath() == name)
					m_Properties.push_back({ nodeEntity, &prop });
			}
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
		m_IsCreated = true;
		return true;
	}
	void AnimationPlayer::Reset()
	{
		m_CurrentTime = 0.0f;
		for (auto& node : m_Properties)
			node.Key = 0;
	}
	void AnimationPlayer::SetCurrentFrame(uint32_t frame)
	{
		XYZ_ASSERT(m_Animation.Raw(), "");
		m_CurrentTime = frame * m_Animation->GetFrameLength();
		for (auto& node : m_Properties)
		{
			node.Key = node.Property->FindKey(frame);
		}
	}
	bool AnimationPlayer::Valid() const
	{
		if (!m_Animation.Raw() || !m_Entity.IsValid())
			return false;

		const ECSManager& ecs = *m_Entity.GetECS();
		const Relationship& rel = m_Entity.GetComponent<Relationship>();
		const std::vector<Entity> tree = rel.GetTree(ecs);

		for (const auto& node : m_Properties)
		{
			const auto& rel = node.Entity.GetComponent<Relationship>();
			if (node.Property->GetPath() != rel.GetPath(*m_Entity.GetECS(), node.Entity, m_Entity))
				return false;
		}
		return true;
	}
	bool AnimationPlayer::Compatible(SceneEntity entity) const
	{
		XYZ_ASSERT(m_Animation.Raw() && entity.IsValid(), "");
		const size_t numProperties = m_Animation->GetPropertyCount();
		const auto& rel = entity.GetComponent<Relationship>();
		const auto& ecs = *entity.GetECS();
		std::vector<Entity> tree = rel.GetTree(ecs);
		tree.insert(tree.begin(), entity);

		size_t numCompatibles = 0;
		for (const Entity node : tree)
		{
			SceneEntity nodeEntity(node, entity.GetScene());
			std::string path = nodeEntity.GetComponent<Relationship>().GetPath(ecs, node, entity);
			numCompatibles += PropertyCompatible(m_Animation->GetProperties<glm::quat>(), path);
			numCompatibles += PropertyCompatible(m_Animation->GetProperties<glm::vec4>(), path);
			numCompatibles += PropertyCompatible(m_Animation->GetProperties<glm::vec3>(), path);
			numCompatibles += PropertyCompatible(m_Animation->GetProperties<glm::vec2>(), path);
			numCompatibles += PropertyCompatible(m_Animation->GetProperties<float>(), path);
			numCompatibles += PropertyCompatible(m_Animation->GetProperties<void*>(), path);
		}

		return numCompatibles == numProperties;
	}
	uint32_t AnimationPlayer::GetCurrentFrame() const
	{
		return static_cast<uint32_t>(std::floor(m_CurrentTime / m_Animation->GetFrameLength()));
	}
}