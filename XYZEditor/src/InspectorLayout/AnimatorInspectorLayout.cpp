#include "AnimatorInspectorLayout.h"


namespace XYZ {
    
    void AnimatorInspectorLayout::OnInGuiRender()
	{   
        if (m_Context && m_Graph)
        {
            if (InGui::GetCurrentWindow()->Flags & InGuiWindowFlag::LeftClicked)
            {
                m_SelectedConnection = -1;
            }
            else if (m_SelectedConnection != -1)
            {
                if (InGui::IsKeyPressed(ToUnderlying(KeyCode::XYZ_KEY_DELETE)))
                {
                    m_Graph->Disconnect((uint32_t)m_SelectedConnection);
                    m_SelectedConnection = -1;
                }
            }
          
            uint32_t count = 0;
            for (auto& connection : m_Graph->GetConnections())
            {     
                std::string result = m_Context->GetStateName(connection.Start) + " ----> " + m_Context->GetStateName(connection.End);
                
                if (m_SelectedConnection == count)
                {
                    InGui::Text(result.c_str(), { 0.8f,0.8f }, { 0,1,1,1 });
                }
                else if (InGui::Text(result.c_str(), { 0.8f,0.8f }))
                {
                    m_SelectedConnection = (int)count;                   
                }
                
                InGui::Separator();
                count++;
            }          
        }
	}
}