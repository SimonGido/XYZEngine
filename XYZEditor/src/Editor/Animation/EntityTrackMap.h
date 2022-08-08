#pragma once

#include "XYZ/ImGui/NeoSequencer/imgui_neo_sequencer.h"

namespace XYZ {
	namespace Editor {
		class EntityTrackMap
		{
		public:
			template <typename T>
			struct TrackData
			{
				std::vector<ImNeoKeyFrame> KeyFrames;
				ImNeoKeyChangeFn		   KeyChangeFunc;
				bool					   Open = true;
			};

			struct EntityData
			{
				std::vector<TrackData<glm::vec4>> Vec4Tracks;
				std::vector<TrackData<glm::vec3>> Vec3Tracks;
				bool Open = true;
			};

			

			std::unordered_map<std::string, EntityData> DataMap;

		};
		
	}
}