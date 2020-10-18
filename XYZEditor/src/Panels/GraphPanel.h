#pragma once
#include <XYZ.h>

namespace XYZ {

	class GraphLayout
	{
	public:
		virtual void OnInGuiRender() {};
		virtual void OnUpdate(Timestep ts) {};
		virtual void OnEvent(Event& event) {};
	};

	class GraphPanel
	{
	public:
		GraphPanel();

		bool OnInGuiRender(Timestep ts);
		void SetGraphLayout(GraphLayout* layout) { m_Layout = layout; }
		void OnEvent(Event& event);
	private:
		bool onWindowResize(WindowResizeEvent& event);

	private:
		GraphLayout* m_Layout = nullptr;
		InGuiWindow* m_GraphWindow = nullptr;
		Ref<FrameBuffer> m_FBO;
		Ref<RenderPass> m_RenderPass;

		EditorCamera m_Camera;
		InGuiMesh m_Mesh;
		InGuiLineMesh m_LineMesh;

		glm::vec2 m_TestPosition = { 0,0 };

		bool m_ActiveWindow = false;
		const uint32_t m_GraphID = 3;
	};
}