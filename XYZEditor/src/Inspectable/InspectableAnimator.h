#pragma once
#include <XYZ.h>

#include "../Panels/InspectorPanel.h"

namespace XYZ {

	class InspectableAnimator : public Inspectable
	{
	public:
		virtual void OnInGuiRender();
		virtual void OnUpdate(Timestep ts);
		virtual void OnEvent(Event& event);
		void SetContext(const Ref<AnimationController>& context);

	private:
		static constexpr glm::vec4 sc_DefaultColor = { 1,1,1,1 };
		static constexpr glm::vec4 sc_SelectColor = { 0.5f, 0.8f, 1.0f, 1.0f };
		static constexpr int32_t sc_InvalidIndex = -1;

		Ref<AnimationController> m_Context = nullptr;
		Graph* m_Graph = nullptr;

		int32_t m_SelectedConnection = sc_InvalidIndex;
		struct Connection
		{
			uint32_t Source = 0;
			uint32_t Destination = 0;
		};
		Connection m_SelectedEdge;
	};
}