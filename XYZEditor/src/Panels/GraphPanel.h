#pragma once
#include <XYZ.h>

namespace XYZ {

	class GraphLayout
	{
	public:
		virtual void OnInGuiRender() {};
	};

	class GraphPanel
	{
	public:
		GraphPanel();

		bool OnInGuiRender(float dt);
		void OnEvent(Event& event);

		void SetGraphLayout(GraphLayout* layout) { m_Layout = layout; }

	private:
		GraphLayout* m_Layout = nullptr;
		InGuiNodeWindow* m_GraphWindow = nullptr;

		const uint32_t m_GraphID = 3;
	};
}