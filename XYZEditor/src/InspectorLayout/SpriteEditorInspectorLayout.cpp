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
				InGui::Separator();
				if (m_Context)
					InGui::Image("Preview", m_Context->GetTexture()->GetRendererID(), { 0,0 }, { 100,100 }, { m_Values[0],m_Values[1],m_Values[2],m_Values[3] });

				InGui::Separator();

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