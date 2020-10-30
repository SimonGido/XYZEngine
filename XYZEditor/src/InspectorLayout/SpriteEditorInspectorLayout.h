#pragma once
#include <XYZ.h>

#include "../Panels/InspectorPanel.h"


namespace XYZ {

	class SpriteEditorInspectorLayout : public InspectorLayout
	{
	public:
		SpriteEditorInspectorLayout();
		virtual void OnInGuiRender() override;
		
		void SetContext(const Ref<SubTexture2D>& context);
	private:
		bool m_FinishedModifying = false;
		bool m_SpriteOpen = false;

		Ref<SubTexture2D> m_Context;

		int m_Selected = -1;
		float m_Values[4];
		int m_Lengths[4] = { 4,4,4,4 };
	};

}