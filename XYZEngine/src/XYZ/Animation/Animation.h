#pragma once
#include "XYZ/Asset/Asset.h"
#include "AnimationTrack.h"

namespace XYZ {
	
	class Animation : public Asset
	{
	public:
		void AddTrack(TrackType type, const std::string& trackName);
		
		size_t TrackCount() const;
		float  FrameLength() const { return 1.0f / Frequency; }
		bool   HasTrack(const std::string_view trackName, TrackType type) const;



		template <typename T>
		constexpr std::vector<AnimationTrack<T>>& GetTracks();

		std::vector<AnimationTrack<glm::vec4>> Vec4Tracks;
		std::vector<AnimationTrack<glm::vec3>> Vec3Tracks;
		std::vector<AnimationTrack<glm::vec2>> Vec2Tracks;
		std::vector<AnimationTrack<float>>	   FloatTracks;

		uint32_t NumFrames;
		uint32_t Frequency;
		bool     Repeat = true;
	};
	template<typename T>
	inline constexpr std::vector<AnimationTrack<T>>& Animation::GetTracks()
	{
		if constexpr (std::is_same_v<T, glm::vec4>)
			return Vec4Tracks;
		else if constexpr (std::is_same_v<T, glm::vec3>)
			return Vec3Tracks;
		else if constexpr (std::is_same_v<T, glm::vec2>)
			return Vec2Tracks;
		else if constexpr (std::is_same_v<T, float>)
			return FloatTracks;
	}
}