#pragma once
#include "XYZ/Editor/EditorUI.h"

#include "XYZ/Renderer/Animation.h"

namespace XYZ {

	namespace Editor {
		class AnimationEditor : public EditorUI
		{
		public:
			AnimationEditor(const std::string& filepath);

			virtual void OnUpdate(Timestep ts) override;
			virtual void OnReload() override;
			virtual void SetupUI() override;

			void SetContext(const Ref<Animation>& context);

			void OnEvent(Event& event);
		private:
			Ref<Animation> m_Context;
		};
	}
}