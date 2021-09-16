#pragma once
#include "XYZ/Asset/Asset.h"
#include "XYZ/Scene/SceneEntity.h"
#include "Property.h"
#include "Animatable.h"
#include "Property.h"

#include <glm/glm.hpp>

#include <any>

namespace XYZ {

	class Animation : public Asset
	{
	public:
		Animation();

		template <typename T>
		void AddAnimatable(const SceneEntity& entity);

		template <typename T>
		void AddProperty(const SceneEntity entity, const SetPropertyRefFn& callback);

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
		std::vector<Animatable>			 m_Animatables;
		std::vector<Property<glm::vec3>> m_Vec3Properties;
		std::vector<Property<std::any>>	 m_PointerProperties;
		std::vector<Property<float>>     m_FloatProperties;


		
		uint32_t m_NumFrames;
		uint32_t m_CurrentFrame;
		float	 m_CurrentTime;
		float    m_FrameLength;
		bool     m_Repeat;
	};
	template<typename T>
	inline void Animation::AddAnimatable(const SceneEntity& entity)
	{
		bool canAdd = m_Animatables.empty();
		for (const auto& it : m_Animatables)
		{
			if (!it.IsType<T>() || it.GetEntity() != entity)
			{
				canAdd = true;
				break;
			}
		}
		XYZ_ASSERT(canAdd, "Animation already has animatable type for entity");
		m_Animatables.emplace_back(entity, [](std::any& ref, SceneEntity ent) {
			ref = &ent.GetComponent<T>();
			
		});
	}
	template<typename T>
	inline void Animation::AddProperty(const SceneEntity entity, const SetPropertyRefFn& callback)
	{
	}
}