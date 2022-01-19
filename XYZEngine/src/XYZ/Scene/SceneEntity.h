#pragma once

#include "Scene.h"
#include "XYZ/Reflection/Reflection.h"

namespace XYZ {

	class SceneEntity
	{
	public:
		SceneEntity();
		SceneEntity(const SceneEntity& other);	
		SceneEntity(Entity id, Scene* scene);
				
		template<typename T>
		T& GetComponent();

		template <typename T>
		const T& GetComponent() const;
		
		template<typename T>
		T& GetComponentFromReflection(Reflection<T> refl);

		template<typename T>
		const T& GetComponentFromReflection(Reflection<T> refl) const;

		template <typename T, typename ...Args>
		std::tuple<Args...> GetComponentTupleFromReflection(Reflection<T> refl);

		template <typename T, typename ...Args>
		const std::tuple<Args...> GetComponentTupleFromReflection(Reflection<T> refl) const;

		template <typename T, typename ...Args>
		T& EmplaceComponent(Args&&... args);
		
		template <typename T>
		T& AddComponent(const T& component);
		
		template <typename T>
		void RemoveComponent();
		
		template <typename T>
		bool HasComponent() const;
		
		template <typename T>
		bool HasComponentFromReflection(Reflection<T> refl) const;

		void Destroy();
		
		bool IsValid() const;
		

		Entity		 ID() const { return m_ID; }

		Scene*		 GetScene() { return m_Scene; }

		const Scene* GetScene() const { return m_Scene; }

		ECSManager*  GetECS();

		const ECSManager* GetECS() const;

	
		SceneEntity& operator =(const SceneEntity& other);
		
		bool operator ==(const SceneEntity& other) const;
		
		operator uint32_t () const { return m_ID; }
		operator Entity() const { return Entity(m_ID); }
	private:
		Scene*   m_Scene;
		Entity   m_ID;


		friend class Scene;
		friend class SceneSerializer;
		friend class ScriptEngine;
	};
	template<typename T>
	inline T& SceneEntity::GetComponent()
	{
		return m_Scene->m_ECS.GetComponent<T>(m_ID);
	}
	template<typename T>
	inline const T& SceneEntity::GetComponent() const
	{
		return m_Scene->m_ECS.GetComponent<T>(m_ID);
	}
	template<typename T>
	inline T& SceneEntity::GetComponentFromReflection(Reflection<T> refl)
	{
		return m_Scene->m_ECS.GetComponent<T>(m_ID);
	}
	template<typename T>
	inline const T& SceneEntity::GetComponentFromReflection(Reflection<T> refl) const
	{
		return m_Scene->m_ECS.GetComponent<T>(m_ID);
	}
	template<typename T, typename ...Args>
	inline std::tuple<Args...> SceneEntity::GetComponentTupleFromReflection(Reflection<T> refl)
	{
		return Reflection<T>::ToReferenceTuple(m_Scene->m_ECS.GetComponent<T>(m_ID));
	}
	template<typename T, typename ...Args>
	inline const std::tuple<Args...> SceneEntity::GetComponentTupleFromReflection(Reflection<T> refl) const
	{
		return Reflection<T>::ToReferenceTuple(m_Scene->m_ECS.GetComponent<T>(m_ID));
	}
	template<typename T, typename ...Args>
	inline T& SceneEntity::EmplaceComponent(Args && ...args)
	{
		return m_Scene->m_ECS.EmplaceComponent<T, Args...>(m_ID, std::forward<Args>(args)...);
	}
	template<typename T>
	inline T& SceneEntity::AddComponent(const T& component)
	{
		return m_Scene->m_ECS.AddComponent<T>(m_ID, component);
	}
	template<typename T>
	inline void SceneEntity::RemoveComponent()
	{
		m_Scene->m_ECS.RemoveComponent<T>(m_ID);
	}
	template<typename T>
	inline bool SceneEntity::HasComponent() const
	{
		return m_Scene->m_ECS.HasComponent<T>(m_ID);
	}
	template<typename T>
	inline bool SceneEntity::HasComponentFromReflection(Reflection<T> refl) const
	{
		return m_Scene->m_ECS.HasComponent<T>(m_ID);
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
