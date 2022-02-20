#include "stdafx.h"
#include "EntityTrackMap.h"

namespace XYZ {
	namespace Editor {
		void EntityTrackMap::BuildMap(Ref<Animation>& anim)
		{
			DataMap.clear();
			for (auto& track : anim->Vec3Tracks)
			{
				std::vector<ImNeoKeyFrame> keyFrames;
				for (auto& keyFrame : track.Keys)
					keyFrames.push_back({ keyFrame.Frame, &keyFrame });
						
				auto& data = DataMap[track.GetName()];
				data.Vec3Tracks.push_back({
					&track,
					keyFrames,
					&keyChangeFunc<glm::vec3>,
					true
				});
			}
		}
	}
}


