#pragma once

#include "Scene.h"

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
		

		template <typename FuncT, typename ...Args>
		void	 AddOnConstruction(FuncT&& func, Args&& ...args);

		template <typename Type>
		void	 AddOnConstruction(void(Type::* func)(), Type* instance);

		template <typename FuncT>
		void	 RemoveOnConstruction(FuncT&& func);

		template <typename Type>
		void	 RemoveOnConstruction(void(Type::* func)(), Type* instance);


		template <typename FuncT, typename ...Args>
		void	 AddOnDestruction(FuncT&& func, Args&& ...args);

		template <typename Type>
		void	 AddOnDestruction(void(Type::* func)(), Type* instance);

		template <typename FuncT>
		void	 RemoveOnDestruction(FuncT&& func);

		template <typename Type>
		void	 RemoveOnDestruction(void(Type::* func)(), Type* instance);



		template <typename T, typename FuncT, typename ...Args>
		void	 AddOnComponentConstruction(FuncT&& func, Args&& ...args);

		template <typename T, typename Type>
		void	 AddOnComponentConstruction(void(Type::* func)(), Type* instance);

		template <typename T, typename FuncT>
		void	 RemoveOnComponentConstruction(FuncT&& func);

		template <typename T, typename Type>
		void	 RemoveOnComponentConstruction(void(Type::* func)(), Type* instance);


		template <typename T, typename FuncT, typename ...Args>
		void	 AddOnComponentDestruction(FuncT&& func, Args&& ...args);

		template <typename T, typename Type>
		void	 AddOnComponentDestruction(void(Type::* func)(), Type* instance);

		template <typename T, typename FuncT>
		void	 RemoveOnComponentDestruction(FuncT&& func);

		template <typename T, typename Type>
		void	 RemoveOnComponentDestruction(void(Type::* func)(), Type* instance);

		Entity		 ID() const { return m_ID; }

		const Scene* GetScene() const { return m_Scene; }

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
		return m_Scene->m_ECS.Contains<T>(m_ID);
	}
	template<typename FuncT, typename ...Args>
	inline void SceneEntity::AddOnConstruction(FuncT&& func, Args && ...args)
	{
		m_Scene->m_ECS.AddOnConstruction(func, std::forward<Args>(args)...);
	}
	template<typename Type>
	inline void SceneEntity::AddOnConstruction(void(Type::* func)(), Type* instance)
	{
		m_Scene->m_ECS.AddOnConstruction(func, instance);
	}
	template<typename FuncT>
	inline void SceneEntity::RemoveOnConstruction(FuncT&& func)
	{
		m_Scene->m_ECS.RemoveOnConstruction(std::forward<FuncT>(func));
	}
	template<typename Type>
	inline void SceneEntity::RemoveOnConstruction(void(Type::* func)(), Type* instance)
	{
		m_Scene->m_ECS.RemoveOnConstruction(func, instance);
	}
	template<typename FuncT, typename ...Args>
	inline void SceneEntity::AddOnDestruction(FuncT&& func, Args && ...args)
	{
		m_Scene->m_ECS.AddOnDestruction(std::forward<FuncT>(func), std::forward<Args>(args)...);
	}
	template<typename Type>
	inline void SceneEntity::AddOnDestruction(void(Type::* func)(), Type* instance)
	{
		m_Scene->m_ECS.AddOnDestruction(func, instance);
	}
	template<typename FuncT>
	inline void SceneEntity::RemoveOnDestruction(FuncT&& func)
	{
		m_Scene->m_ECS.RemoveOnDestruction(std::forward<FuncT>(func));
	}
	template<typename Type>
	inline void SceneEntity::RemoveOnDestruction(void(Type::* func)(), Type* instance)
	{
		m_Scene->m_ECS.RemoveOnDestruction(func, instance);
	}
	template<typename T, typename FuncT, typename ...Args>
	inline void SceneEntity::AddOnComponentConstruction(FuncT&& func, Args && ...args)
	{
		m_Scene->m_ECS.AddOnConstruction(std::forward<FuncT>(func), std::forward<Args>(args)...);
	}
	template<typename T, typename Type>
	inline void SceneEntity::AddOnComponentConstruction(void(Type::* func)(), Type* instance)
	{
		m_Scene->m_ECS.AddOnComponentConstruction<T, Type>(func, instance);
	}
	template<typename T, typename FuncT>
	inline void SceneEntity::RemoveOnComponentConstruction(FuncT&& func)
	{
		m_Scene->m_ECS.RemoveOnComponentConstruction<T, FuncT>(std::forward<FuncT>(func));
	}
	template<typename T, typename Type>
	inline void SceneEntity::RemoveOnComponentConstruction(void(Type::* func)(), Type* instance)
	{
		m_Scene->m_ECS.RemoveOnComponentConstruction<T, Type>(func, instance);
	}
	template<typename T, typename FuncT, typename ...Args>
	inline void SceneEntity::AddOnComponentDestruction(FuncT&& func, Args && ...args)
	{
		m_Scene->m_ECS.AddOnComponentDestruction<T, FuncT, Args>(std::forward<FuncT>(func), std::forward<Args>(args)...);
	}
	template<typename T, typename Type>
	inline void SceneEntity::AddOnComponentDestruction(void(Type::* func)(), Type* instance)
	{
		m_Scene->m_ECS.AddOnComponentDestruction<T, Type>(func, instance);
	}
	template<typename T, typename FuncT>
	inline void SceneEntity::RemoveOnComponentDestruction(FuncT&& func)
	{
		m_Scene->m_ECS.RemoveOnComponentDestruction<T, FuncT>(std::forward<FuncT>(func));
	}
	template<typename T, typename Type>
	inline void SceneEntity::RemoveOnComponentDestruction(void(Type::* func)(), Type* instance)
	{
		m_Scene->m_ECS.RemoveOnComponentDestruction<T, Type>(func, instance);
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
