#pragma once

#include "XYZ/Scene/SceneEntity.h"

namespace XYZ {
	template <typename T>
	struct KeyFrame
	{
		bool operator <(const KeyFrame<T>& other) const
		{
			return Frame < other.Frame;
		}
		bool operator >(const KeyFrame<T>& other) const
		{
			return Frame > other.Frame;
		}

		T		 Value;
		uint32_t Frame = 0;
	};

	enum class TrackDataType
	{
		None, Float, Vec2, Vec3, Vec4
	};

	enum class TrackType
	{
		None,
		Translation, 
		Rotation,
		Scale
	};

	inline std::string TrackTypeToString(TrackType type)
	{
		switch (type)
		{
		case TrackType::None:		 return "None";
		case TrackType::Translation: return "Translation";
		case TrackType::Rotation:	 return "Rotation";
		case TrackType::Scale:		 return "Scale";
		}
		XYZ_ASSERT(false, "");
		return "";
	}

	inline TrackDataType TrackTypeToDataType(TrackType type)
	{
		switch (type)
		{
		case TrackType::None:		 return TrackDataType::None;
		case TrackType::Translation: return TrackDataType::Vec3;
		case TrackType::Rotation:	 return TrackDataType::Vec3;
		case TrackType::Scale:		 return TrackDataType::Vec3;
		}
		return TrackDataType::None;
	}

	template <typename T>
	inline constexpr TrackDataType TypeToTrackDataType()
	{
		if constexpr (std::is_same_v<T, glm::vec4>)
			return TrackDataType::Vec4;
		if constexpr (std::is_same_v<T, glm::vec3>)
			return TrackDataType::Vec3;
		if constexpr (std::is_same_v<T, glm::vec2>)
			return TrackDataType::Vec2;
		if constexpr (std::is_same_v<T, float>)
			return TrackDataType::Float;
		return TrackDataType::None;
	}

	template <typename T>
	class AnimationTrack
	{
	public:
		AnimationTrack(TrackType type, const std::string& name);

		void			   Update(size_t key, uint32_t frame, T& value);
		TrackType		   GetTrackType()		   const { return m_Type; }
		size_t			   GetKey(uint32_t frame)  const;
		const std::string& GetName()			   const { return m_Name; }

		static constexpr TrackDataType GetTrackDataType() { return TypeToTrackDataType<T>(); }

		std::vector<KeyFrame<T>> Keys;
	private:
		TrackType     m_Type;
		std::string   m_Name;
	};
	template<typename T>
	inline AnimationTrack<T>::AnimationTrack(TrackType type, const std::string& name)
		:
		m_Type(type),
		m_Name(name)
	{
	}

	template<typename T>
	inline size_t AnimationTrack<T>::GetKey(uint32_t frame) const
	{
		if (!Keys.empty())
		{
			for (size_t i = 0; i < Keys.size() - 1; ++i)
			{
				const auto& key = Keys[i];
				const auto& next = Keys[i + 1];
				if (frame >= key.Frame && frame < next.Frame)
					return i;
			}
			return Keys.size() - 1;
		}
		return -1;
	}

}