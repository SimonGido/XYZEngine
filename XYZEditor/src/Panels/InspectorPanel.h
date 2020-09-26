#pragma once
#include <XYZ.h>

namespace XYZ {

	class InspectorLayout
	{
	public:
		virtual void OnInGuiRender() {};
	};

	class InspectorPanel
	{
	public:
		InspectorPanel();
		void OnInGuiRender();
		void SetInspectorLayout(InspectorLayout* layout) { m_Layout = layout; }


	private:
		InspectorLayout* m_Layout = nullptr;
	};
}