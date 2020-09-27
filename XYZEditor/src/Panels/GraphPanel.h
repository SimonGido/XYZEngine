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

		void OnInGuiRender(float dt);
		void SetGraphLayout(GraphLayout* layout) { m_Layout = layout; }


	private:
		GraphLayout* m_Layout = nullptr;

		const uint32_t m_GraphID = 3;
	};
}