#pragma once
#include "XYZ/Animation/Animation.h"
#include "AnimationSequencer.h"

namespace XYZ {
	namespace Editor {

		class AnimationEditor
		{
		public:
			AnimationEditor();

			void SetContext(const Ref<Animation>& context);
			void SetScene(const Ref<Scene>& scene);

			void OnUpdate(Timestep ts);
			void OnImGuiRender(bool& open);
		private:
			void handleSelected(bool addKey);
			void addTransformTrack(const SceneEntity& entity);

		private:
			Ref<Animation>	   m_Context;
			Ref<Scene>		   m_Scene;

			AnimationSequencer m_Sequencer;
			int				   m_SelectedEntry;
			int				   m_FirstFrame;
			int				   m_CurrentFrame;
			bool			   m_Expanded;
			bool			   m_Playing;
		};
	}
}