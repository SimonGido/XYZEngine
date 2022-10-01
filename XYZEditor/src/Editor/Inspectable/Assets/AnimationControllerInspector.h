#pragma once
#include "Editor/Inspectable/Inspectable.h"
#include "XYZ/Asset/Animation/AnimationController.h"


namespace XYZ {
	namespace Editor {
		class AnimationControllerInspector : public Inspectable
		{
		public:
			AnimationControllerInspector();

			virtual bool OnEditorRender() override;
			virtual void SetAsset(const Ref<Asset>& asset) override;

			virtual Type GetType() const { return Type::Asset; }

		private:
			void handleSkeleton();
			void handleAnimations();
		private:
			Ref<AnimationController> m_ControllerAsset;
		};
	}
}