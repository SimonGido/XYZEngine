#pragma once
#include "XYZ/Asset/Asset.h"
#include "XYZ/Scene/SceneEntity.h"
#include "Property.h"
#include "Animatable.h"
#include "Property.h"

#include <glm/glm.hpp>

#include <unordered_set>

namespace XYZ {

	class Animation : public Asset
	{
	public:
		Animation();
		virtual ~Animation() override;

		template <typename ComponentType, typename ValueType>
		void AddProperty(SceneEntity entity, const std::string& valueName);

		template <typename ComponentType, typename ValueType>
		void RemoveProperty(const SceneEntity& entity, const std::string& valueName);

		void SetActiveScene(const Ref<Scene>& scene);

		void Update(Timestep ts);
		void UpdateLength();
		void SetCurrentFrame(uint32_t frame);
		void SetNumFrames(uint32_t numFrames) { m_NumFrames = numFrames; }
		void SetRepeat(bool repeat)			  { m_Repeat = repeat; }

		inline uint32_t						  GetNumFrames()   const { return m_NumFrames; }
		inline uint32_t						  GetTime()        const { return m_CurrentFrame; }
		inline float						  GetFrameLength() const { return m_FrameLength; }
		inline bool							  GetRepeat()	   const { return m_Repeat; }
	
	private:
		template <typename ValueType>
		Property<ValueType>& addPropertySpecialized(const Property<ValueType>& prop);
		
		template <typename ValueType>
		void				 removePropertySpecialized(const SceneEntity& entity, const std::string& valueName, const std::string& componentName);
		
		template <typename T>
		static void			 removeFromContainer(std::vector<T>& container, const SceneEntity& entity, const std::string& valueName, const std::string& componentName);

		void updatePropertyReferences();
		void clearProperties();
	private:
		Ref<Scene>						 m_ActiveScene;

		// TODO: Registration of components counter, add callback by component ID instead of Type
		std::vector<uint32_t>			 m_RegisteredComponents;

		std::vector<Property<glm::vec4>> m_Vec4Properties;
		std::vector<Property<glm::vec3>> m_Vec3Properties;
		std::vector<Property<glm::vec2>> m_Vec2Properties;
		std::vector<Property<float>>     m_FloatProperties;
		std::vector<Property<void*>>	 m_PointerProperties;
		
		uint32_t m_NumFrames;
		uint32_t m_CurrentFrame;
		float	 m_CurrentTime;
		float    m_FrameLength;
		bool     m_Repeat;
	};
	

	template<typename ComponentType, typename ValueType>
	inline void Animation::AddProperty(SceneEntity entity, const std::string& valueName)
	{
		XYZ_ASSERT(m_ActiveScene.Raw(), "No active scene");
		XYZ_ASSERT(entity.GetScene() == m_ActiveScene.Raw(), "Entity does not belong to the active scene");
		SetPropertyRefFn<ValueType> callback = [](SceneEntity ent, ValueType* ref, const std::string& varName) {
			ref = &Reflection<ComponentType>::GetByName<ValueType>(varName.c_str(), ent.GetComponent<ComponentType>());
		};
		addPropertySpecialized<ValueType>(Property<ValueType>(callback, entity, valueName, Reflection<ComponentType>::GetClassName()));
	}

	template<typename ComponentType, typename ValueType>
	inline void Animation::RemoveProperty(const SceneEntity& entity, const std::string& valueName)
	{
		XYZ_ASSERT(m_ActiveScene.Raw(), "No active scene");
		XYZ_ASSERT(entity.GetScene() == m_ActiveScene.Raw(), "Entity does not belong to the active scene");
		removePropertySpecialized<ValueType>(entity, valueName, Reflection<ComponentType>::GetClassName());
	}

	template<typename T>
	void Animation::removeFromContainer(std::vector<T>& container, const SceneEntity& entity, const std::string& valueName, const std::string& componentName)
	{
		for (size_t i = 0; i < container.size(); ++i)
		{
			auto& prop = container[i];
			if (prop.GetSceneEntity() == entity && prop.GetValueName() == valueName && prop.GetComponentName() == componentName)
			{
				container.erase(container.begin() + i);
				return;
			}
		}
	}
}