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
			void OnImGuiRender(bool& open);
		
		private:
			void handleSelected(bool addKey);
			void addTransformTrack();

		private:
			Ref<Animation>			m_Context;

			AnimationSequencer m_Sequencer;
			int				   m_SelectedEntry;
			int				   m_FirstFrame;
			int				   m_CurrentFrame;
			bool			   m_Expanded;
		};
	}
}