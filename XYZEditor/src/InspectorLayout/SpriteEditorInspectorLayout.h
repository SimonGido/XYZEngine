#pragma once
#include <XYZ.h>

#include "../Panels/InspectorPanel.h"


namespace XYZ {

	class SpriteEditorInspectorLayout : public InspectorLayout
	{
	public:
		virtual void OnInGuiRender() override;
		
		void SetContext(const Ref<SubTexture2D>& context);
	private:
		bool m_FinishedModifying = false;
		bool m_SpriteOpen = false;

		bool m_XModified = false;
		bool m_YModified = false;
		bool m_ZModified = false;
		bool m_WModified = false;

		std::string m_X;
		std::string m_Y;
		std::string m_Z;
		std::string m_W;

		Ref<SubTexture2D> m_Context;
	};

}