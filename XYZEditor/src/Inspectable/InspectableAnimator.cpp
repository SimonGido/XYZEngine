#include "InspectableAnimator.h"

namespace XYZ {
    void InspectableAnimator::OnInGuiRender()
    {
        if (m_Context)
        {
            auto& machine = m_Context->GetStateMachine();
            auto currentState = machine.GetCurrentState();
            for (uint32_t j = 0; j < machine.GetNumStates(); ++j)
            {
                if (machine.IsStateInitialized(j))
                {
                    auto& destination = machine.GetState(j);
                    if (currentState.CanTransitTo(destination.GetID()))
                    {
                        std::string result = machine.GetStateName(currentState.GetID()) + " ----> " + machine.GetStateName(destination.GetID());
                        glm::vec4 color = sc_DefaultColor;
                        if (m_SelectedConnection == j)
                            color = sc_SelectColor;
                        if (InGui::Text(result.c_str(), color) & InGuiReturnType::Clicked)
                        {
                            m_SelectedConnection = j;
                            m_SelectedEdge.Source = currentState.GetID();
                            m_SelectedEdge.Destination = destination.GetID();
                        }
                        InGui::Separator();
                    }
                }
            }        
        }
    }
    void InspectableAnimator::OnUpdate(Timestep ts)
    {
    }
    void InspectableAnimator::OnEvent(Event& event)
    {
    }
    void InspectableAnimator::SetContext(const Ref<AnimationController>& context)
    {
        m_Context = context;
    }
}