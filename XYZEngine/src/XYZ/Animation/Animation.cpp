#include "stdafx.h"
#include "Animation.h"


namespace XYZ {
	void Animation::AddTrack(TrackType type, const std::string& trackName)
	{
		TrackDataType dataType = TrackTypeToDataType(type);
		if (dataType == TrackDataType::Vec4)
			Vec4Tracks.emplace_back(type, trackName);
		else if (dataType == TrackDataType::Vec3)
			Vec3Tracks.emplace_back(type, trackName);
		else if (dataType == TrackDataType::Vec2)
			Vec2Tracks.emplace_back(type, trackName);
		else if (dataType == TrackDataType::Float)
			FloatTracks.emplace_back(type, trackName);
	}
	size_t Animation::TrackCount() const
	{
		return Vec4Tracks.size() + Vec3Tracks.size() + Vec2Tracks.size();
	}
	bool Animation::HasTrack(const std::string_view trackName, TrackType type) const
	{
		for (auto& track : Vec4Tracks)
		{
			if (track.GetName() == trackName && track.GetTrackType() == type)
				return true;
		}
		for (auto& track : Vec3Tracks)
		{
			if (track.GetName() == trackName && track.GetTrackType() == type)
				return true;
		}
		for (auto& track : Vec2Tracks)
		{
			if (track.GetName() == trackName && track.GetTrackType() == type)
				return true;
		}
		for (auto& track : FloatTracks)
		{
			if (track.GetName() == trackName && track.GetTrackType() == type)
				return true;
		}
		return false;
	}
}