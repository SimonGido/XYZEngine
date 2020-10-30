#include "AnimatorInspectorLayout.h"


namespace XYZ {
    AnimatorInspectorLayout::AnimatorInspectorLayout()
    {
    }
    void AnimatorInspectorLayout::OnInGuiRender()
	{   
        if (m_Context && m_Graph)
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
                                    m_SelectedEdge.Source = source.GetID();
                                    m_SelectedEdge.Destination = destination.GetID();
                                }
                                InGui::Separator();
                            }
                        }
                    }
                }
            }
            if (InGui::ResolveLeftClick(false))
                m_SelectedConnection = sc_InvalidIndex;
        }
	}

    void AnimatorInspectorLayout::OnEvent(Event& event)
    {
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<KeyPressedEvent>(Hook(&AnimatorInspectorLayout::onKeyPress, this));
    }
 
    void AnimatorInspectorLayout::SetGraph(Graph* graph)
    {
        m_Graph = graph;
    }

    void AnimatorInspectorLayout::SetContext(const Ref<AnimationController>& context)
    {
        m_Context = context; 
    }
    bool AnimatorInspectorLayout::onKeyPress(KeyPressedEvent& event)
    {
        if (event.IsKeyPressed(KeyCode::XYZ_KEY_DELETE))
        {
            if (m_SelectedConnection != sc_InvalidIndex)
            {
                m_Context->GetStateMachine().GetState(m_SelectedEdge.Source).DisallowTransition(m_SelectedEdge.Destination);      
                m_SelectedConnection = sc_InvalidIndex;
                m_Graph->RemoveEdge((int32_t)m_SelectedEdge.Source, (int32_t)m_SelectedEdge.Destination);
                
                return true;
            }
        }
        return false;
    }
}