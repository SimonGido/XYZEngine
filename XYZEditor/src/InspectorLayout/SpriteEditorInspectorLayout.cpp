#include "stdafx.h"
#include "SpriteEditorInspectorLayout.h"


namespace XYZ {
	void SpriteEditorInspectorLayout::OnInGuiRender()
	{
		if (m_Context)
		{
			if (InGui::BeginGroup("Sprite", { 0,0 }, m_SpriteOpen))
			{
				InGui::Text("Position", { 0.7f,0.7f });
				
				bool modyfing = false;
				if (InGui::TextArea("X", m_X, { 60.0f, 25.0f }, m_XModified))
				{
					m_FinishedModifying = true;		
					modyfing = true;
				}
				if (InGui::TextArea("Y", m_Y, { 60.0f, 25.0f }, m_YModified))
				{
					m_FinishedModifying = true;
					modyfing = true;
				}
				if (InGui::TextArea("Z", m_Z, { 60.0f, 25.0f }, m_ZModified))
				{
					m_FinishedModifying = true;
					modyfing = true;
				}
				if (InGui::TextArea("W", m_W, { 60.0f, 25.0f }, m_WModified))
				{
					m_FinishedModifying = true;
					modyfing = true;
				}
				
					
				if (m_FinishedModifying && !modyfing)
				{
					m_FinishedModifying = false;
					auto texCoords = m_Context->GetTexCoords();
				
					texCoords.x = atof(m_X.c_str());
					texCoords.y = atof(m_Y.c_str());
					texCoords.z = atof(m_Z.c_str());
					texCoords.w = atof(m_W.c_str());

					m_Context->SetCoords(texCoords);
				}

				InGui::Image("Context", m_Context->GetTexture()->GetRendererID(), { 150,150 }, { 0,0 }, m_Context->GetTexCoords());
			}
		}
	}
	void SpriteEditorInspectorLayout::SetContext(const Ref<SubTexture2D>& context)
	{
		if (context)
		{		
			auto texCoords = m_Context->GetTexCoords();

			m_X = std::to_string(texCoords.x).substr(0, 5);
			m_Y = std::to_string(texCoords.y).substr(0, 5);
			m_Z = std::to_string(texCoords.z).substr(0, 5);
			m_W = std::to_string(texCoords.w).substr(0, 5);
		}
		m_Context = context;
	}
}