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
		bool OnInGuiRender();
		void SetInspectorLayout(InspectorLayout* layout);


	private:
		InspectorLayout* m_Layout = nullptr;
	};
}