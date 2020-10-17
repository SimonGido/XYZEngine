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
		static constexpr glm::vec4 sc_DefaultColor = { 1,1,1,1 };
		static constexpr glm::vec4 sc_SelectColor = { 0.5f, 0.8f, 1.0f, 1.0f };
		static constexpr int32_t sc_InvalidIndex = -1;

		Ref<AnimationController> m_Context = nullptr;

		int32_t m_SelectedConnection = sc_InvalidIndex;

	
	};
}