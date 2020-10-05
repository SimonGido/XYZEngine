#include "stdafx.h"
#include "SpriteEditorInspectorLayout.h"


namespace XYZ {
	void SpriteEditorInspectorLayout::OnInGuiRender()
	{
		if (m_Context)
		{
			if (InGui::BeginGroup("Sprite", { 0,0 }, m_SpriteOpen))
			{
				bool modyfing = false;
				if (InGui::Float(4, "Position", m_Values, m_Lengths, {}, { 60.0f,25.0f }, m_Selected))
				{
					m_FinishedModifying = true;
					modyfing = true;
				}
					
				if (m_FinishedModifying && !modyfing)
				{
					m_FinishedModifying = false;
					auto texCoords = m_Context->GetTexCoords();
				
					texCoords.x = m_Values[0];
					texCoords.y = m_Values[1];
					texCoords.z = m_Values[2];
					texCoords.w = m_Values[3];

					m_Context->SetCoords(texCoords);
				}

				InGui::Image("Context", m_Context->GetTexture()->GetRendererID(), { 0,0 }, { 150,150 },  m_Context->GetTexCoords());
			}
		}
	}
	void SpriteEditorInspectorLayout::SetContext(const Ref<SubTexture2D>& context)
	{
		if (context)
		{		
			auto texCoords = context->GetTexCoords();

			m_Values[0] = texCoords.x;
			m_Values[1] = texCoords.y;
			m_Values[2] = texCoords.z;
			m_Values[3] = texCoords.w;
		}
		m_Context = context;
	}
}