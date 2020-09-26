#pragma once
#include "../Panels/GraphPanel.h"

namespace XYZ {

	class AnimatorGraphLayout : public GraphLayout
	{
	public:
		virtual void OnInGuiRender() override;

		// TODO: probably create controller component
		void SetContext(StateMachine* context, Graph* graph) { m_Context = context; m_Graph = graph; }
	private:
		StateMachine* m_Context = nullptr;
		Graph* m_Graph = nullptr;;

		glm::vec2 m_PopupPosition = { 0,0 };
		bool m_PopupEnabled = false;
		bool m_PopupOpen = false;
	};
}