#pragma once

#include "XYZ/Animation/Animation.h"
#include "XYZ/ImGui/NeoSequencer/imgui_neo_sequencer.h"

namespace XYZ {
	namespace Editor {
		class EntityTrackMap
		{
		public:
			template <typename T>
			struct TrackData
			{
				AnimationTrack<T>*		   Track;
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

			void BuildMap(Ref<Animation>& anim);


			std::unordered_map<std::string, EntityData> DataMap;

		private:
			template <typename T>
			static void keyChangeFunc(const ImNeoKeyFrame* key);

		};
		template<typename T>
		inline void EntityTrackMap::keyChangeFunc(const ImNeoKeyFrame* key)
		{
			KeyFrame<T>* userKey = static_cast<KeyFrame<T>*>(key->UserData);
			userKey->Frame = key->Frame;
		}
	}
}