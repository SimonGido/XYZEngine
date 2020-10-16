#include "AnimatorInspectorLayout.h"


namespace XYZ {

    void AnimatorInspectorLayout::OnInGuiRender()
	{   
        if (m_Context)
        {
            if (InGui::GetCurrentWindow()->Flags & InGuiWindowFlag::LeftClicked)
            {
                m_SelectedConnection = -1;
            }
            else if (m_SelectedConnection != -1)
            {
                if (InGui::IsKeyPressed(ToUnderlying(KeyCode::XYZ_KEY_DELETE)))
                {
                    m_SelectedConnection = -1;
                }
            }

            auto& machine = m_Context->GetStateMachine();
            for (auto& [index, pairN] : machine.GetStatesMap())
            {
                for (auto& [index2, pairK] : machine.GetStatesMap())
                {
                    auto& source = pairN.State;
                    auto& destination = pairK.State;
                    if (source.CanTransitTo(destination.GetID()))
                    {
                        std::string result = machine.GetStateName(source.GetID()) + " ----> " + machine.GetStateName(destination.GetID());
                        InGui::Text(result.c_str(), { 0.8f,0.8f });
                        InGui::Separator();
                    }
                }
            }
        }
	}
    void AnimatorInspectorLayout::SetContext(const Ref<AnimationController>& context)
    {
        m_Context = context; 
    }
}