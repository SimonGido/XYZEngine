#include "AnimatorInspectorLayout.h"


namespace XYZ {

    void AnimatorInspectorLayout::OnInGuiRender()
	{   
        if (m_Context)
        {
            auto& machine = m_Context->GetStateMachine();
            for (uint32_t i = 0; i < machine.GetNumStates(); ++i)
            {
                if (machine.IsStateInitialized(i))
                {
                    for (uint32_t j = 0; j < machine.GetNumStates(); ++j)
                    {
                        if (machine.IsStateInitialized(j))
                        {
                            auto& source = machine.GetState(i);
                            auto& destination = machine.GetState(j);
                            if (source.CanTransitTo(destination.GetID()))
                            {
                                std::string result = machine.GetStateName(source.GetID()) + " ----> " + machine.GetStateName(destination.GetID());
                                glm::vec4 color = sc_DefaultColor;
                                if (m_SelectedConnection == machine.GetNumStates() * j + i)
                                    color = sc_SelectColor;
                                if (InGui::Text(result.c_str(), color) & InGuiReturnType::Clicked)
                                {
                                    m_SelectedConnection = machine.GetNumStates() * j + i;
                                }
                                InGui::Separator();
                            }
                        }
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