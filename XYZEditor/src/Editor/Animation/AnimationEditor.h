#pragma once
#include "Editor/EditorPanel.h"
#include "EntityTrackMap.h"



namespace XYZ {
	namespace Editor {
		class AnimationEditor : public EditorPanel
		{
		public:
			AnimationEditor(std::string name);

			virtual void OnImGuiRender(bool& open) override;
			virtual void OnUpdate(Timestep ts) override;
			virtual void SetSceneContext(const Ref<Scene>& scene) override;


		private:
			void propertySection();
			void timelineSection();


		

		};

	}
}