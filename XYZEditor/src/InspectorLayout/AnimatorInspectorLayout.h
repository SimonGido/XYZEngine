#pragma once
#include <XYZ.h>

#include "../Panels/InspectorPanel.h"


namespace XYZ {

	class AnimatorInspectorLayout : public InspectorLayout
	{
	public:
		virtual void OnInGuiRender() override;
		void SetContext(const Ref<AnimationController>& context);


	private:
		Ref<AnimationController> m_Context = nullptr;

		int32_t m_SelectedConnection = -1;
	};
}