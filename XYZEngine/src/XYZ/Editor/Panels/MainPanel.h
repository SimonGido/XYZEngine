#pragma once
#include "XYZ/BasicUI/BasicUI.h"

namespace XYZ {

	class MainPanel
	{
	public:
		MainPanel();
		~MainPanel();

		void OnUpdate();
		void OnEvent(Event& event);
	private:
		float findPerWindowWidth();

	private:
		bUILayout m_ViewLayout;
	};
}