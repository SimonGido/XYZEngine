#pragma once
#include <XYZ.h>

namespace XYZ {

	class InspectorLayout
	{
	public:
		virtual void OnInGuiRender() {};
		virtual void OnEvent(Event& event) {};
	};

	class InspectorPanel
	{
	public:
		InspectorPanel();
		bool OnInGuiRender();
		void SetInspectorLayout(InspectorLayout* layout);
		void OnEvent(Event& event);

	private:
		InspectorLayout* m_Layout = nullptr;
		InGuiWindow* m_Window;
	};
}