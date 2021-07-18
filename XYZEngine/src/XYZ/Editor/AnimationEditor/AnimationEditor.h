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
			Ref<Animation> m_Context;

			AnimationSequencer m_Sequencer;
		};
	}
}