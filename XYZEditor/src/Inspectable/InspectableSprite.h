#pragma once

#include "Inspectable.h"


namespace XYZ {

	class InspectableSprite : public Inspectable
	{
	public:
		virtual void OnInGuiRender();
		virtual void OnUpdate(Timestep ts);
		virtual void OnEvent(Event& event);

		void SetContext(const Ref<SubTexture2D>& context);
	private:
		Ref<SubTexture2D> m_Context;

		bool m_FinishedModifying = false;
		bool m_SpriteOpen = false;
	
		int32_t m_Selected = -1;
		float m_Values[4];
		int32_t m_Lengths[4] = { 4,4,4,4 };
	};
}