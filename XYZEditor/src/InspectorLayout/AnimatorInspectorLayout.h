#pragma once
#include <XYZ.h>

#include "../Panels/InspectorPanel.h"


namespace XYZ {

	class AnimatorInspectorLayout : public InspectorLayout
	{
	public:
		virtual void OnInGuiRender() override;
		void SetContext(StateMachine* context, Graph* graph) { m_Context = context; m_Graph = graph; }


	private:
		StateMachine* m_Context = nullptr;
		Graph* m_Graph = nullptr;

		int m_SelectedConnection = -1;
	};
}