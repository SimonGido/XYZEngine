#pragma once

#include "Scene.h"
#include "XYZ/Reflection/Reflection.h"

#include <entt/entt.hpp>

namespace XYZ {

	class SceneEntity
	{
	public:
		SceneEntity();
		SceneEntity(const SceneEntity& other);	
		SceneEntity(entt::entity id, Scene* scene);
				
		template<typename T>
		T& GetComponent();

		template <typename T>
		const T& GetComponent() const;
		

		template <typename T, typename ...Args>
		T& EmplaceComponent(Args&&... args);
		
		template <typename T>
		T& AddComponent(const T& component);
		
		template <typename T>
		void RemoveComponent();
		
		template <typename T>
		bool HasComponent() const;
		
		void Destroy();
		
		bool IsValid() const;
		

		entt::entity ID() const { return m_ID; }

		Scene*				  GetScene() { return m_Scene; }
		const Scene*		  GetScene() const { return m_Scene; }
		entt::registry*		  GetRegistry();
		const entt::registry* GetRegistry() const;

	
		SceneEntity& operator =(const SceneEntity& other);
		
		bool operator ==(const SceneEntity& other) const;
		
		operator uint32_t () const { return static_cast<uint32_t>(m_ID); }
	private:
		Scene*		 m_Scene;
		entt::entity m_ID;


		friend class Scene;
		friend class SceneSerializer;
		friend class ScriptEngine;
	};
	template<typename T>
	inline T& SceneEntity::GetComponent()
	{
		return m_Scene->m_Registry.get<T>(m_ID);
	}
	template<typename T>
	inline const T& SceneEntity::GetComponent() const
	{
		return m_Scene->m_Registry.get<T>(m_ID);
	}
	
	template<typename T, typename ...Args>
	inline T& SceneEntity::EmplaceComponent(Args && ...args)
	{
		return m_Scene->m_Registry.emplace<T>(m_ID, std::forward<Args>(args)...);
	}
	template<typename T>
	inline T& SceneEntity::AddComponent(const T& component)
	{
		T& newComp = m_Scene->m_Registry.emplace<T>(m_ID, component);
		return newComp;
	}
	template<typename T>
	inline void SceneEntity::RemoveComponent()
	{
		m_Scene->m_Registry.remove<T>(m_ID);
	}
	template<typename T>
	inline bool SceneEntity::HasComponent() const
	{
		return m_Scene->m_Registry.any_of<T>(m_ID);
	}
}

namespace std {
	// TODO: two entities with the same ID but different Scene are going to return same hash
	template<>
	struct hash<XYZ::SceneEntity>
	{
		std::size_t operator()(XYZ::SceneEntity entity) const noexcept
		{
			const std::hash<uint32_t> hasher;
			return hasher(static_cast<uint32_t>(entity.ID()));
		}
	};
}
