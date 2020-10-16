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
		bool onWindowResize(WindowResizeEvent& event);

	private:
		GraphLayout* m_Layout = nullptr;
		InGuiWindow* m_GraphWindow = nullptr;
		Ref<FrameBuffer> m_FBO;

		bool m_ActiveWindow = false;
		const uint32_t m_GraphID = 3;
	};
}